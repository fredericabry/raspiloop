#include "playback_loop_c.h"
#include <qdebug.h>
#include "QElapsedTimer"
#include "parameters.h"
#include "interface.h"


playback_loop_c::playback_loop_c(int id,  playback_port_c *pPort, long length,bool autoplay):id(id),pPort(pPort)
{
    consumer = NULL;

    filename = QString::number(id)+".wav";

    pPrevLoop = pNextLoop = NULL;



    if(autoplay) status = STATUS_PLAY;
    else status = STATUS_IDLE;//at first the loop is not playing.


    consumer = new playbackLoopConsumer;
    consumer->controler = this;
    consumer->active = true;
    loopConnected = false;
    loopReadyToStop = false;




    buffile = (short*)malloc(sizeof(short)*NFILE_PLAYBACK);
    memset(buffile,0,sizeof(short)*NFILE_PLAYBACK);
    maxlength=RINGBUFSIZE_PLAYBACK;
    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    memset(ringbuf,0,(maxlength+1)*sizeof(short));
    bufsize = pPort->bufsize;
    buf = (short*)malloc(bufsize*sizeof(short));
    memset(buf,0,bufsize*sizeof(short));
    tail =0;
    head = 0;

    frametoplay = (length*RATE)/1000;


    if(frametoplay==0)
    {
        stop = false;
        repeat = false;
    }
    else if(frametoplay<0)
    {
        stop = false;
        repeat = true;

    }
    else
    {
        stop = true;
        repeat = false;
    }


    pPort->addloop(this);



    consumer->start();



}


void playback_loop_c::openFile()
{
    SF_INFO sf_info;


    QString filename2 = DIRECTORY+filename;    buf = (short*)malloc((bufsize)*sizeof(short));

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


}

void playback_loop_c::destroy()
{


    if(!consumer) {qDebug()<<"bug";return;}

    consumer->stop(); //consumer is going to stop and then destroy the loop

}

void playback_loop_c::pushN(short *buf_in, unsigned long N)
{
    short *pt ;

    playloop_mutex.lock();



    if (this->freespace() == 0) return;

    if(N > this->freespace())  { qDebug()<<"not enough space in playback loop ringbuf";playloop_mutex.unlock(); return;}

    if(N > this->maxlength) {qDebug()<<"Failed to copy to ringbuf struct";playloop_mutex.unlock(); return;}

    if(this->head >= this->tail)
    {
        if(N+this->head <= this->maxlength)
        {
            pt = this->ringbuf+(this->head)/*sizeof(short)*/;
            memcpy(pt,buf_in,N*sizeof(short));
            this->head += N;
        }
        else
        {

            //first let us copy the part that fits
            int first = this->maxlength+1 - this->head; //maxlength + 1 because of the additionnal value in the buffer
            pt = this->ringbuf+(this->head)/*sizeof(short)*/;
            memcpy(pt,buf_in,first*sizeof(short));
            //then what remains
            int second = N-first;
            pt = this->ringbuf;
            memcpy(pt,buf_in+first/*sizeof(short)*/,second*sizeof(short));

            this->head = second;

        }
    }
    else
    {

        pt = this->ringbuf+(this->head)/*sizeof(short)*/;
        memcpy(pt,buf_in,N*sizeof(short));
        this->head += N;
    }
    playloop_mutex.unlock();


}

int playback_loop_c::pullN(unsigned long N)
{

    playloop_mutex.lock();

    short *pt ;

    unsigned long length = this->length();

    if((length == 0)&&(loopReadyToStop)) consumer->stop(); //file is empty, ringbuffer is empty, we can start destroying the loop


    if(N > length) {
        qDebug()<<"not enough elements";
        N = length;
    }

    if((this->head < this->tail)&&(this->tail+N>this->maxlength))
    {
        //first let us copy the part that fits
        int first = this->maxlength+1 - this->tail; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->ringbuf+(this->tail)/*sizeof(short)*/;
        memcpy(this->buf,pt,first*sizeof(short));

        //then what remains
        int second = N-first;
        pt = this->ringbuf;

        memcpy(this->buf+first/*sizeof(short)*/,pt,second*sizeof(short));
        this->tail = second;


    }
    else
    {
        pt = this->ringbuf+(this->tail)/*sizeof(short)*/;
        memcpy(this->buf,pt,N*sizeof(short));
        this->tail += N;

    }

    playloop_mutex.unlock();

    return N;
}


unsigned long playback_loop_c::length()
{
    if(this->head>=this->tail) return this->head-this->tail;
    else return this->maxlength-this->tail + this->head + 1;

}

unsigned long playback_loop_c::freespace()
{

    return this->maxlength-this->length();

}





void playback_loop_c::play()
{

    this->status = STATUS_PLAY;

}

void playback_loop_c::pause()
{

    this->status = STATUS_IDLE;

}

void playback_loop_c::moveToPort(playback_port_c *pNuPort)
{

    if(!pNuPort) {qDebug()<<"invalid port";return;}
    pPort->removeloop(this);
    pPort = pNuPort;
    pPort->addloop(this);

}







playback_loop_c::~playback_loop_c(void)
{
    free(buffile);

    free(ringbuf);

    free(buf);



}

void playback_loop_c::rewind(void)
{
    sf_seek(soundfile,0,SFM_READ);
}






void playback_loop_c::datarequest(unsigned long frames)
{

    int nread;

    //first request only activate the loop to avoid synchronization issues
    if(!loopConnected)
        return;

    if(frames > bufsize)
    frames = bufsize;


    //the associated playback port requests more data

    if(status == STATUS_IDLE)
    {

        //pausing, not sending any data
        emit send_data(buf,0);//we still need to answer to the data request or the playback port get stuck waiting for data
        return;

    }

    int ringbuflength = length();

    if(frames > ringbuflength) frames = ringbuflength; //limit data transfert to what is available in the ringbuffer




    nread = pullN(frames);



   // qDebug()<<frames<<nread;


    if (nread != frames) qDebug()<<"error pulling from ringbuf";



    if(this->stop)
    {


        this->frametoplay -= nread;
    }

    emit send_data(buf,nread);







}

void playback_loop_c::activate()
{
    loopConnected = true;
}


void playbackLoopConsumer::run()
{
    update_lock = false;
    controler->openFile();


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
   // qDebug()<<"upd";
    update_lock = true;
    int nread,nrequest,t1;
    static int tmax = 0;


    sf_command (controler->soundfile, SFC_UPDATE_HEADER_NOW, NULL, 0) ; //update the file length, we might be writing on it in some other thread


    nrequest =controler->freespace();
    //qDebug()<<nrequest;
    if(nrequest > NFILE_PLAYBACK) nrequest = NFILE_PLAYBACK;

    t3.start();

    if((nread = sf_readf_short(controler->soundfile,controler->buffile,nrequest))>0)
    {


        controler->pushN(controler->buffile,nread);

    }




     if((controler->stop&&(controler->frametoplay<=0))
             ||((nread <= 0)&&(nrequest!=0)))
    {

        if(controler->repeat)
        {
            sf_seek(controler->soundfile,0,SFM_READ);

        }
        else
        {
           // qDebug()<<"stop";
          controler->loopReadyToStop = true;
          // controler->destroyloop(true);
            //qDebug()<<"stop";

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

        controler->pPort->removeloop(controler);

    }

    delete controler;


}
