#include "capture_loop_c.h"
#include "playback_loop_c.h"
#include "qdir.h"
#include "QDebug"
#include "interface.h"
#include "events.h"
#include "click_c.h"


capture_loop_c::capture_loop_c(const int id, capture_port_c *pPort,  long length, bool createPlayLoop,std::vector<playback_port_c*>pPlayPorts, double delta):id(id),pPort(pPort)
{



    filename = QString::number(id)+".wav";

    framesCount = 0;
    barCount = oldBarCount = 0;
    //pPort->interface->listMutex.lock();
    addToList();
    //pPort->interface->listMutex.unlock();

    connect(this,SIGNAL(makeInterfaceEvent(const QObject*,const char*,int,void*,bool,interfaceEvent_c**)),pPort->interface,SLOT(createInterfaceEvent(const QObject*,const char*,int,void*,bool,interfaceEvent_c**)));


    long offset = delta*RATE; //number of elements recorded before we need to copy
    if(offset > RATE) {qDebug()<<"too late"; offset = RATE;}
    long x = (signed long)pPort->head - offset;
    while(x < 0) x+=pPort->maxlength+1;
    tail = (long)x;



    buffile = (short*)malloc(CAPTURE_LOOP_BUFSIZE*sizeof(short));
    memset(buffile,0,CAPTURE_LOOP_BUFSIZE*sizeof(short));
    this->openfile(filename);

    //length value in ms
    if(length>0)
    {
        stop = true;
        framesToRead= (length*RATE)/1000;
    }
    else
    {
        stop = false;

    }




    consumer = new captureLoopConsumer();
    consumer->controler = this;
    recording = true;
    consumer->start();


    pPlayLoop = NULL;

    if(createPlayLoop)
    {
        //we need to create an associated playbackloop, which will be ready

        if(pPort->interface->synchroMode == NOSYNC)
        {


            //  pPlayLoop = new playback_loop_c(id,pPlayPorts,playPortsCount,-1,NOSYNC,IDLE,pPort->interface);//by default loop
        }
        else if(pPort->interface->synchroMode == CLICKSYNC)
        {



            playData_s *param;
            param = new playData_s;
            param->id = id;
            param->length = 1;//one bar
            param->pPlayPorts = pPlayPorts;
            param->skipevent = 0;
            param->status = HIDDEN;
            param->syncMode = CLICKSYNC;
            param->pCaptureLoop = this;

            emit makeInterfaceEvent(pPort->interface->pClick,SIGNAL(firstBeat()),EVENT_CREATE_PLAY,(void*)param,false,&pEvent);


        }




    }
    else pPlayLoop = NULL;



}

capture_loop_c::~capture_loop_c()
{
    free(buffile);



}

void capture_loop_c::addToList(void)
{



    pPort->interface->captureListMutex.lock();



    pNextLoop = NULL;//last event created


    pPrevLoop = pPort->interface->findLastCaptureLoop();



    if(pPort->interface->firstCaptureLoop == NULL) pPort->interface->firstCaptureLoop = this;
    else if(pPrevLoop)
    {
        pPrevLoop->pNextLoop = this;
    }
    else qDebug()<<"bug addtolist capture";



    pPort->interface->captureListMutex.unlock();

    pPort->interface->printCaptureLoopList();

}

void capture_loop_c::removeFromList(void)
{

    pPort->interface->captureListMutex.lock();

    if(pPrevLoop == NULL) pPort->interface->firstCaptureLoop = pNextLoop;//it was the first loop, let's update this info

    if(pPrevLoop) pPrevLoop->pNextLoop = pNextLoop; //if it was not the first loop
    if(pNextLoop) pNextLoop->pPrevLoop = pPrevLoop; //it was not the last loop

    pPort->interface->captureListMutex.unlock();

    pPort->interface->printCaptureLoopList();
}

void capture_loop_c::destroyLoop()
{




    if(pPort->interface->synchroMode == NOSYNC)
    {

        if(pPlayLoop != NULL)
            pPlayLoop->play();//start

    }
    else if((pPort->interface->synchroMode == CLICKSYNC))
    {

        //we need to compute the length of the loop in beats,
        double beatsCountf = (double)pPort->interface->pClick->getTempo()*framesCount/(RATE*60);
        long beatsCount = (long)beatsCountf;
        int nbeats,nbars;
        nbeats=nbars = 0;

        while(nbeats<beatsCount) nbeats+=4;
        nbeats-=4;

        if(beatsCount-nbeats >= 2) //recording was stopped up to 2 beats too early, let us make the recording one bar longer
            nbeats+=4;

        nbars = nbeats/4;

        if(nbars==0) nbars = 1;//length was smaller than 1 bar, let's not ignore what has been recorded


        qDebug()<<beatsCountf<<"beats -"<<nbars<<"bars";

        if(pPlayLoop != NULL)
        {

            pPlayLoop->updateFrameToPlay(nbars);
            pPlayLoop->play();

        }
        else
        {

            if((pEvent)&&(pPort->interface->isEventValid(pEvent))) {pEvent->playData->status=PLAY;pEvent->playData->pCaptureLoop=NULL;} // we need to update the event infos
            else  emit updatePlayLoopInfo(nbars);//event already created the loop, let us provide the infos directly to the loop


        }




    }

    //this loop needs to get out of the list:


    removeFromList();



    recording = false;


}

int capture_loop_c::pullN(unsigned long N)
{

    short *pt ;
    unsigned long length ;


    unsigned long nuTail = this->tail;

    pPort->ring_lock.lock();
    unsigned long nuHead = this->pPort->head;
    //pPort->ring_lock.unlock();

    if(nuHead>=nuTail) length = nuHead-nuTail;
    else length = pPort->maxlength-nuTail + nuHead + 1;

    if(length == 0) {pPort->ring_lock.unlock();return 0;}


    if(N > length)
        N = length;


    if((nuHead < nuTail)&&(nuTail+N>this->pPort->maxlength+1))
    {

        //first let us copy the part at the end of the ringbuffer
        unsigned long first = this->pPort->maxlength+1 - nuTail; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->pPort->ringbuf+(nuTail)/*sizeof(short)*/;
        memcpy(this->buffile,pt,first*sizeof(short));
        //then what remains
        unsigned long second = N-first;
        pt = this->pPort->ringbuf;
        memcpy(this->buffile+first/*sizeof(short)*/,pt,second*sizeof(short));
        nuTail = second;

    }
    else
    {

        pt = this->pPort->ringbuf+(nuTail)/*sizeof(short)*/;
        memcpy(this->buffile,pt,N*sizeof(short));
        nuTail += N;

    }



    this->tail = nuTail;
    pPort->ring_lock.unlock();
    return N;
}

void capture_loop_c::openfile(QString filename)
{

    SF_INFO sf_info;
    int short_mask;

    sf_info.samplerate = RATE;
    sf_info.channels = 1;

    short_mask = SF_FORMAT_PCM_16;

    sf_info.format = SF_FORMAT_WAV|short_mask;

    QDir r;

    filename = DIRECTORY+filename;

    r.remove(filename);//in case there is already a file named like that

    // filedir = filename.remove(filename.length()-3,3) + "tmp";
    filedir = filename;


    const char * fn = filedir.toStdString().c_str();

    int cmpt = 0;
    while ((soundfile = sf_open (filedir.toStdString().c_str(), SFM_WRITE, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile \"%s\" for input (%s)\n",fn, errstr);

        cmpt++;

        qDebug()<<cmpt;

        if(cmpt>100) exit(1);
    }



}

void capture_loop_c::closefile()
{
    int err;
    if ((err = sf_close(soundfile))) {
        qDebug()<<"cannot close sndfile  for output error# "<< err;

        // exit (1);
    }
}

void captureLoopConsumer::update(void)
{

    int t1;
    static int tmax = 0;
    int nread,err;

    if((controler->stop)&&(controler->framesToRead<NFILE_CAPTURE))
        nread = controler->pullN(controler->framesToRead);//just what we still have to read
    else
        nread = controler->pullN(NFILE_CAPTURE);


    if(nread >0 )
    {

        telapsed.start();
        if ((err = sf_writef_short (controler->soundfile, controler->buffile,  nread) ) != nread)   qDebug()<< "cannot write sndfile"<<err;

        t1 = telapsed.elapsed();
        if(t1 > tmax) {tmax = t1; qDebug()<<"new save delay max : "<<tmax<<" ms";}

        // if (t1 > 2) qDebug()<<"save delay: "<<t1<<"ms max : "<<tmax<<" ms"<<nread;

        controler->framesCount += nread;//let's keep track of how many frames have been read

       controler->barCount =controler->pPort->interface->pClick->getTempo()*controler->framesCount/(RATE*60*4);

       if(controler->barCount!=controler->oldBarCount)
       {
        controler->oldBarCount = controler->barCount;
        controler->pPort->interface->printCaptureLoopList();

       }


    }

    if(controler->stop)
    {

        controler->framesToRead -= nread;

        if(controler->framesToRead <= 0)//time to stop
        {
            controler->destroyLoop();

        }

    }




}

void captureLoopConsumer::run()
{

    while(1)
    {
        if(!controler->recording) break;
        update();
        QThread::usleep(CAPTURE_WRITEFILE_SLEEP);
    }

    controler->closefile();
    //delete controler;
    controler->deleteLater();
    //   qDebug()<<"capture loop consumer destroyed";

}


