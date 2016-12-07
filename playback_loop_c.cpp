#include "playback_loop_c.h"
#include <qdebug.h>
#include "QElapsedTimer"
#include "parameters.h"
#include "interface.h"
#include "click_c.h"
#include "events.h"

playback_loop_c::playback_loop_c(int id,  std::vector<playback_port_c*>pPorts, long length,syncoptions syncMode,status_t status,interface_c *interface):id(id),pPorts(pPorts),syncMode(syncMode),status(status),interface(interface)
{


    if((id == 0) || (id==1)) isClick = true;
    else isClick = false;


    pPrevLoop = pNextLoop = NULL;

    consumer = NULL;
    filename = QString::number(id)+".wav";

    connect(this,SIGNAL(makeInterfaceEvent(const QObject*,const char*,int,void*,bool,interfaceEvent_c**)),interface,SLOT(createInterfaceEvent(const QObject*,const char*,int,void*,bool,interfaceEvent_c**)));

    consumer = new playbackLoopConsumer;
    consumer->controler = this;
    consumer->active = true;

    loopReadyToStop = false;

    buffile = (short*)malloc(sizeof(short)*NFILE_PLAYBACK);
    memset(buffile,0,sizeof(short)*NFILE_PLAYBACK);
    maxlength=RINGBUFSIZE_PLAYBACK;
    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    memset(ringbuf,0,(maxlength+1)*sizeof(short));
    bufsize = PLAYBACK_BUFSIZE;


    head = 0;

    updateFrameToPlay(length);

    if(syncMode == CLICKSYNC)
    {
        restartplayData_s *restartplayData = new restartplayData_s;
        restartplayData->id = id;
        restartplayData->pLoop = this;
        restartplayData->skipevent=0;
        restartplayData->status=PLAY;
        makeInterfaceEvent(interface->pClick,SIGNAL(firstBeat()),EVENT_PLAY_RESTART,(void*)restartplayData,true,&pEvent);
    }



    std::fill(framesCount.begin(),framesCount.end(),0);
    isOutOfSample = false;






    //qDebug()<<"size:"<<pPorts.size();

    //multi port support : each playback port to which the loop is connected has its own tail and framescount
    for (auto &pPort : pPorts)
    {
        portsChannel.push_back(pPort->channel);
        tails.push_back(head);
        framesCount.push_back(0);
        loopConnectedToPort.push_back(false);
        pPort->addloop(this);

    }


    addToList();

    consumer->start();


}


void playback_loop_c::openFile()
{
    SF_INFO sf_info;


    QString filename2 = DIRECTORY+filename;

    isFileOpened = false;

    if ((this->soundfile = sf_open (filename2.toStdString().c_str(), SFM_RDWR, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile \"%s\" for output (%s)\n",filename.toStdString().c_str(), errstr);


        destroy();
        return;
    }

    isFileOpened = true;
    //   qDebug()<<"created 2 "<<id;


    int short_mask = SF_FORMAT_PCM_16;


    if(sf_info.format != (SF_FORMAT_WAV|short_mask))
    {
        qDebug()<<"format de fichier incorrect\n";
        destroy();
        return;
    }
    /*
    if(sf_info.samplerate != rate_play)
    {
        parent_play->Afficher("soundfile rate incorrect\n");
        return;
    }
    */
    if(sf_info.channels != 1)
    {
        qDebug()<<"chan nbr incorrect\n";
        destroy();
        return;
    }

    /* if(id>1)
      qDebug()<<framestoplay<<sf_info.frames;*/



}

void playback_loop_c::destroy()
{

    if(!consumer) {qDebug()<<"no consumer bug";return;}

    consumer->stop(); //consumer is going to stop and then destroy the loop

}

void playback_loop_c::pushN(short *buf_in, unsigned long N)
{
    short *pt ;

    playloop_mutex.lock();



    if (this->freespace() == 0) {playloop_mutex.unlock(); return;}

    if(N > this->freespace())  { qDebug()<<"not enough space in playback loop ringbuf";playloop_mutex.unlock(); return;}

    if(N > this->maxlength) {qDebug()<<"Failed to copy to ringbuf struct";playloop_mutex.unlock(); return;}

    /* if(this->head >= this->tail)
    {
        if(N+this->head <= this->maxlength)
        {
            pt = this->ringbuf+(this->head);
            memcpy(pt,buf_in,N*sizeof(short));
            this->head += N;
        }
        else
        {

            //first let us copy the part that fits
            int first = this->maxlength+1 - this->head; //maxlength + 1 because of the additionnal value in the buffer
            pt = this->ringbuf+(this->head);
            memcpy(pt,buf_in,first*sizeof(short));
            //then what remains
            int second = N-first;
            pt = this->ringbuf;
            memcpy(pt,buf_in+first,second*sizeof(short));

            this->head = second;

        }
    }
    else
    {

        pt = this->ringbuf+(this->head);
        memcpy(pt,buf_in,N*sizeof(short));
        this->head += N;
    }

    */

    if(N+this->head <= this->maxlength)
    {
        pt = this->ringbuf+(this->head);
        memcpy(pt,buf_in,N*sizeof(short));
        this->head += N;
    }
    else
    {

        //first let us copy the part that fits
        int first = this->maxlength+1 - this->head; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->ringbuf+(this->head);
        memcpy(pt,buf_in,first*sizeof(short));
        //then what remains
        int second = N-first;
        pt = this->ringbuf;
        memcpy(pt,buf_in+first,second*sizeof(short));

        this->head = second;

    }





    playloop_mutex.unlock();


}

int playback_loop_c::pullN(unsigned long N,int portNumber,short **buf)
{



    playloop_mutex.lock();

    short *pt ;

    unsigned long length = this->length(tails[portNumber]);

    if((length == 0)&&(loopReadyToStop)) {consumer->stop();playloop_mutex.unlock();return 0;} //file is empty, ringbuffer is empty, we can start destroying the loop




    if(N > length) {
        qDebug()<<"not enough elements";
        N = length;
    }

    if((this->head < tails[portNumber])&&(tails[portNumber]+N>this->maxlength))
    {
        //first let us copy the part that fits
        int first = this->maxlength+1 - tails[portNumber]; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->ringbuf+(tails[portNumber])/*sizeof(short)*/;
        memcpy(*buf,pt,first*sizeof(short));

        //then what remains
        int second = N-first;
        pt = this->ringbuf;

        memcpy(*buf+first/*sizeof(short)*/,pt,second*sizeof(short));
        tails[portNumber] = second;


    }
    else
    {
        pt = this->ringbuf+(tails[portNumber])/*sizeof(short)*/;
        memcpy(*buf,pt,N*sizeof(short));
        tails[portNumber] += N;

    }

    playloop_mutex.unlock();



    return N;
}


unsigned long playback_loop_c::length(unsigned long tail2)
{
    if(this->head>=tail2) return this->head-tail2;
    else return this->maxlength-tail2 + this->head + 1;

}

unsigned long playback_loop_c::freespace()
{
    unsigned long minfreespace,temp;

    minfreespace = maxlength;

    for (auto &tail_t : tails)
    {
        temp = this->maxlength-this->length(tail_t);
        if(temp<minfreespace) minfreespace = temp;

    }

    return minfreespace;

    //  return this->maxlength-this->length();

}

void playback_loop_c::play()
{
    this->status = PLAY;

    interface->printLoopList();
}

void playback_loop_c::pause()
{
    if(syncMode == CLICKSYNC)
        status = SILENT;
    else

        this->status = IDLE;


    interface->printLoopList();
}

bool playback_loop_c::addToPortList(playback_port_c *pNuPort)
{

    for (auto &pPort : pPorts)
    {

        if(pPort->channel == pNuPort->channel) {qDebug()<<"playback port already in the loop list";return false;}

    }


    pNuPort->addloop(this);
    portsChannel.push_back(pNuPort->channel);
    pPorts.push_back(pNuPort);
    tails.push_back(head);
    framesCount.push_back(0);

    interface->printLoopList();

    return true;

}

bool playback_loop_c::removeFromPortList(playback_port_c *pOldPort)
{

    int n = -1;
    for (unsigned int i =0;i<pPorts.size();i++)
    {

        if(pPorts[i]->channel == pOldPort->channel) {n = i;break;}

    }

    if(n==-1) {qDebug()<<"playback port not in the loop list";return false;}


    pOldPort->removeloop(this);
    portsChannel.erase(portsChannel.begin()+n);
    pPorts.erase(pPorts.begin()+n);
    tails.erase(tails.begin()+n);
    framesCount.erase(framesCount.begin()+n);

    interface->printLoopList();

    return true;

}

void playback_loop_c::moveToPort(playback_port_c *pNuPort)
{
    if(!pNuPort) {qDebug()<<"invalid port";return;}

    //this->addToPortList(pNuPort);




    /*  pPort->removeloop(this);
    pPort = pNuPort;
    pPort->addloop(this);*/
}

void playback_loop_c::updateFrameToPlay(long length)
{



    if(syncMode == NOSYNC)
    {

        framestoplay = (length*RATE)/1000;
        if(framestoplay==0)
        {
            stop = false;
            repeat = false;
        }
        else if(framestoplay<0)
        {
            stop = false;
            repeat = true;

        }
        else
        {
            stop = true;
            repeat = false;
        }


    }
    else if(syncMode == CLICKSYNC)
    {
        //length is given in bars
        barstoplay = length;
        stop = false;
        framestoplay = length*4*RATE*60/interface->pClick->getTempo();

    }





}

void playback_loop_c::infoFromCaptureLoop(unsigned long length)
{

    updateFrameToPlay(length);
    play();


}

playback_loop_c::~playback_loop_c(void)
{

    free(buffile);
    free(ringbuf);


    //  qDebug()<<"playback loop destroyed";
}

void playback_loop_c::rewind(void)
{
    sf_seek(soundfile,0,SFM_READ);

    std::fill(framesCount.begin(),framesCount.end(),0);
    isOutOfSample = false;

}

int playback_loop_c::findCapturePortNumber(int channel)
{
    int portNumber = -1;
    for (unsigned int j = 0;j<portsChannel.size();j++)
    {
        if(portsChannel[j]==channel) {portNumber=j;break;}


    }
    return portNumber;

}


void playback_loop_c::datarequest(unsigned long frames,int channel)
{

    int nread;

    //find out which playbackport is requesting data
    int portNumber = findCapturePortNumber(channel);
    if(portNumber == -1) {qDebug()<<"data request port channel unknown"<<channel;return;}


    //first request only activate the loop to avoid synchronization issues
    if(!loopConnectedToPort[portNumber])
    {qDebug()<<"bug connected to port";
        return;
    }

    if(frames > bufsize)
        frames = bufsize;




    short *buf2 = new short[bufsize];

    //the associated playback port requests more data

    if((status == IDLE)||
            (isOutOfSample))//we chose to stop or the interface is in clicksync mode and the play loop awaits rewind
    {


        //pausing, not sending any data

        emit send_data(buf2,0,channel);//we still need to answer to the data request otherwise the playback port gets stuck waiting for data


        return;

    }

    unsigned long ringbuflength = length(tails[portNumber]);

    if(frames > ringbuflength) frames = ringbuflength; //limit data transfert to what is available in the ringbuffer


    // nread = pullN(frames);
    nread = pullN(frames,portNumber,&buf2);



    if (nread != (signed long)frames) qDebug()<<"error pulling from ringbuf";


    framesCount[portNumber]+=nread;


    if(status == SILENT) memset(buf2,0,nread*sizeof(short));




    emit send_data(buf2,nread,channel);



}

void playback_loop_c::activate(int channel)
{
    int portNumber = findCapturePortNumber(channel);
    if(portNumber == -1) {qDebug()<<"activate port channel unknown"<<channel;return;}

    loopConnectedToPort[portNumber] = true;

    if(!disconnect(pPorts[portNumber]->consumer,SIGNAL(update_loops(int)), this, SLOT(activate(int))))
        qDebug()<<"update loop disconnection failed";

}

void playback_loop_c::addToList(void)
{
    if(isClick) return;//this is the click, we don't keep it in the loops list
    interface->playbackListMutex.lock();


    pNextLoop = NULL;//last loop created
    pPrevLoop = interface->findLastPlaybackLoop();


    if(interface->firstPlayLoop == NULL) interface->firstPlayLoop = this;
    else if(pPrevLoop)
    {

        pPrevLoop->pNextLoop = this;
    }
    interface->playbackListMutex.unlock();

    interface->printLoopList();
}

void playback_loop_c::removeFromList(void)
{
    if(isClick) return;//this is the click, we don't keep it in the loops list


    interface->playbackListMutex.lock();
    if(pPrevLoop == NULL) interface->firstPlayLoop = pNextLoop;//it was the first loop, let's update this info

    if(pPrevLoop) pPrevLoop->pNextLoop = pNextLoop; //if it was not the first loop
    if(pNextLoop) pNextLoop->pPrevLoop = pPrevLoop; //it was not the last loop

    interface->playbackListMutex.unlock();



    interface->printLoopList();


}

bool playback_loop_c::isFinished(void)
{

    for (unsigned int i = 0;i<framesCount.size();i++)
    {


        if((signed)framesCount[i]>=framestoplay) return true;

    }
    return false;
}




//CONSUMER

void playbackLoopConsumer::run()
{
    update_lock = false;
    controler->openFile();

    controler->rewind();

    while(1)
    {

        if(!active) break;
        if(update_lock) {return;}
        update();
        QThread::usleep(CAPTURE_READFILE_SLEEP);
    }

    destroyloop();


}

QElapsedTimer t3;

void playbackLoopConsumer::update() //constantly fill the ringbuffer with data from the opened file
{

    if(controler->loopReadyToStop) return;//we don't need more data

    // qDebug()<<"upd";
    update_lock = true;
    int nread,nrequest,t1;
    static int tmax = 0;


    sf_command (controler->soundfile, SFC_UPDATE_HEADER_NOW, NULL, 0) ; //update the file length, we might be writing on it in some other thread



    nrequest =controler->freespace();

    if(nrequest > NFILE_PLAYBACK) nrequest = NFILE_PLAYBACK;

    t3.start();

    if((nread = sf_readf_short(controler->soundfile,controler->buffile,nrequest))>0)
    {


        controler->pushN(controler->buffile,nread);

    }


    //qDebug()<<controler->stop<<controler->framestoplay;

    if(controler->syncMode == NOSYNC)
    {

        if((controler->stop&&(controler->isFinished()))
                ||((nread <= 0)&&(nrequest!=0)))
        {


            if(controler->repeat)
            {
                for (auto &pPort : controler->pPorts)
                    pPort->removeloop(controler);
                sf_seek(controler->soundfile,0,SFM_READ);

            }
            else
            {

                controler->loopReadyToStop = true;


            }



        }


    }
    else if(controler->syncMode == CLICKSYNC)
    {

        if(      ( (controler->isFinished()) ||((nread <= 0)&&(nrequest!=0)) ))

        {

            if(!controler->stop)controler->isOutOfSample = true;//we are not stopping yet so we just need to freeze data transfert
            else  controler->loopReadyToStop = true;





        }



    }



    t1 = t3.elapsed();

    if(t1 > tmax) tmax = t1;
    if (t1 > 50) qDebug()<<"load delay: "<<t1<<"ms max : "<<tmax<<" ms"<<nread;
    update_lock = false;


}

void playbackLoopConsumer::stop(void)
{

    active = false;

}

void playbackLoopConsumer::destroyloop()
{
    if (controler->isFileOpened)
    {

        sf_close(controler->soundfile);


        for (auto &pPort : controler->pPorts)
            pPort->removeloop(controler);

        controler->removeFromList();
    }
    controler->deleteLater();

}
