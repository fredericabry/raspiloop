#include "capture_loop_c.h"
#include "qdir.h"
#include "QDebug"

#define READ_OFFSET (signed long)3000

capture_loop_c::capture_loop_c(const QString id,const QString filename,capture_port_c *pPort,const int rate, const unsigned long bufsize):id(id),filename(filename),pPort(pPort),rate(rate),bufsize(bufsize)
{

    long x = (signed long)pPort->head - READ_OFFSET;
    while(x < 0) x+=pPort->maxlength;
    tail = x;


    buffile = (short*)malloc(bufsize*sizeof(short));
    memset(buffile,0,bufsize*sizeof(short));

    this->openfile(filename);

    consumer = new captureLoopConsumer();
    consumer->port = this;
    recording = true;
    consumer->start();

}

capture_loop_c::~capture_loop_c()
{
    free(buffile);

}

void capture_loop_c::destroyLoop()
{
    recording = false;
    closefile();
    delete this;
}

int capture_loop_c::pullN(unsigned long N)
{

    short *pt ;
    unsigned long length ;


    unsigned long nuTail = this->tail;

    pPort->ring_lock.lock();
    unsigned long nuHead = this->pPort->head;
    pPort->ring_lock.unlock();

    if(nuHead>=nuTail) length = nuHead-nuTail;
    else length = pPort->maxlength-nuTail + nuHead + 1;

    if(length == 0) {return 0;}


    if(N > length)
        N = length;


    if((nuHead < nuTail)&&(nuTail+N>this->pPort->maxlength))
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

    return N;
}

void capture_loop_c::openfile(QString filename)
{

    SF_INFO sf_info;
    int short_mask;

    sf_info.samplerate = rate;
    sf_info.channels = 1;

    short_mask = SF_FORMAT_PCM_16;

    sf_info.format = SF_FORMAT_WAV|short_mask;

    QDir r;

    filename = DIRECTORY+filename;

    r.remove(filename);//in case there is already a file named like that

    filedir = filename.remove(filename.length()-3,3) + "tmp";

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

    QDir r;


    QString old = filedir;
    filedir.replace(filedir.length()-3,3,"wav");

    if(!r.rename(old,filedir)) qDebug()<<"cannot rename file";
}

void captureLoopConsumer::update(void)
{
    int t1;
    static int tmax = 0;
    int nread,err;

    nread = port->pullN(NFILE_CAPTURE);


    if(nread >0 )
    {
        telapsed.start();

        if ((err = sf_writef_short (port->soundfile, port->buffile,  nread) ) != nread)   qDebug()<< "cannot write sndfile"<<err;

        t1 = telapsed.elapsed();
        if(t1 > tmax) tmax = t1;

        if (t1 > 50) qDebug()<<"save delay: "<<t1<<"ms max : "<<tmax<<" ms";

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

}


