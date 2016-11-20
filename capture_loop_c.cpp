#include "capture_loop_c.h"
#include "playback_loop_c.h"
#include "qdir.h"
#include "QDebug"
#include "interface.h"
#include "events.h"
#include "click_c.h"

#define READ_OFFSET (signed long)500

capture_loop_c::capture_loop_c(const int id, capture_port_c *pPort,  long length, bool createPlayLoop, playback_port_c *pPlayPort):id(id),pPort(pPort)
{

    pPlayLoop = NULL;


    pPrevLoop = pPort->interface->findLastCaptureLoop();
    pNextLoop = NULL;//last loop created.

    filename = QString::number(id)+".wav";

    if(pPort->interface->firstCaptureLoop == NULL)
        pPort->interface->firstCaptureLoop = this;
    else
        pPrevLoop->pNextLoop = this;//let's add it at the end of the chain



    long x = (signed long)pPort->head - READ_OFFSET;
    while(x < 0) x+=pPort->maxlength+1;
    tail = x;

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
    consumer->port = this;
    recording = true;
    consumer->start();


    if(createPlayLoop)
    {
        //we need to create an associated playbackloop, which will be ready
       // pPlayLoop = new playback_loop_c(id,pPlayPort,-1,false);//by default loop
        playData params;
        params.autoplay = false;
        params.id = this->id;
        params.length = -1;
        params.pPlayPort = pPlayPort;
        new interfaceEvent_c(pPort->interface->pClick,SIGNAL(firstBeat()),pPort->interface->findLastEvent(),EVENT_PLAY,(void*)&params,pPort->interface);


    }
    else pPlayLoop = NULL;


}

capture_loop_c::~capture_loop_c()
{
    free(buffile);

    //    qDebug()<<"capture loop destroyed";

}

void capture_loop_c::destroyLoop()
{

    //let's start the associated playback_loop :
    pPlayLoop = pPort->interface->findPlayLoopById(id);
    if(pPlayLoop != NULL)
      {
        pPlayLoop->play();

    }


    //this loop needs to get out of the list:
    if(pPrevLoop==NULL) pPort->interface->firstCaptureLoop = pNextLoop;//it was the first loop
    else
    {
        pPrevLoop->pNextLoop = pNextLoop;
    }

    if(pNextLoop!=NULL) //there is a next loop
    {
        pNextLoop->pPrevLoop = pPrevLoop;
    }


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

    if((port->stop)&&(port->framesToRead<NFILE_CAPTURE))
        nread = port->pullN(port->framesToRead);//just what we still have to read
    else
        nread = port->pullN(NFILE_CAPTURE);


    if(nread >0 )
    {

        telapsed.start();
        if ((err = sf_writef_short (port->soundfile, port->buffile,  nread) ) != nread)   qDebug()<< "cannot write sndfile"<<err;

        t1 = telapsed.elapsed();
        if(t1 > tmax) {tmax = t1; qDebug()<<"new save delay max : "<<tmax<<" ms";}

        // if (t1 > 2) qDebug()<<"save delay: "<<t1<<"ms max : "<<tmax<<" ms"<<nread;

    }

    if(port->stop)
    {

        port->framesToRead -= nread;

        if(port->framesToRead <= 0)//time to stop
        {
            port->destroyLoop();

        }

    }




}

void captureLoopConsumer::run()
{

    while(1)
    {
        if(!port->recording) break;
        update();
        QThread::usleep(CAPTURE_WRITEFILE_SLEEP);
    }

    port->closefile();
    delete port;
    //   qDebug()<<"capture loop consumer destroyed";

}


