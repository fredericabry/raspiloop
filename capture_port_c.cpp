#include "capture_port_c.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_capture.h"
#include <stdbool.h>
#include <sndfile.h>
#include "capture_port_c.h"
#include <QMainWindow>
#include <QFile>
#include <QTextStream>

#include "qdir.h"
#include "qlockfile.h"

#include <qdebug.h>
#include <qelapsedtimer.h>
#include "qthreadpool.h"




capture_port_c::capture_port_c(const unsigned long maxlength, const unsigned long bufsize, const int rate, const int id):maxlength(maxlength),bufsize(bufsize),rate(rate),id(id)
{

    ring_lock.lock();

    this->head = 0;
    this->tail = 0;

    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    memset(ringbuf,0,maxlength*sizeof(short));
    bufin = (short*)malloc((bufsize)*sizeof(short));
    memset(bufin,0,bufsize*sizeof(short));
    buffile = (short*)malloc((NFILE_CAPTURE)*sizeof(short));
    memset(buffile,0,NFILE_CAPTURE*sizeof(short));


    recording = false;
    consumer = new Consumer();
    consumer->port = this;
    ring_lock.unlock();

    consumer->start();



}

capture_port_c::~capture_port_c()
{

    consumer->quit();
    free(ringbuf);
    free(bufin);
    free(buffile);

}


unsigned long capture_port_c::length()
{

    if(this->head>=this->tail) return this->head-this->tail;
    else return this->maxlength-this->tail + this->head + 1;

}

unsigned long capture_port_c::freespace()
{
    return this->maxlength-this->length();
}


void capture_port_c::pushN(unsigned long N)
{


    static bool fg_full = false;
    short *pt ;
    int test;

    ring_lock.lock();


    unsigned long freespace = this->freespace();
    if(N > freespace)  {
        if(recording)
        {
            if(!fg_full)
            {
                qDebug()<<"capture buffer full";
                fg_full = true;
            }
        }
        //not enough space, we need to forget N-freespace samples

        this->freeN(N-freespace);

    }else fg_full = false;

    unsigned long nuHead = this->head;
    unsigned long nuTail = this->tail;

    ring_lock.unlock();


    test = 0;


    if(N > this->maxlength) {qDebug()<<"Failed to copy to capture ringbuf struct";  return;}

    if(nuHead >= nuTail)
    {
        if(N+nuHead <= this->maxlength)
        {
            pt = this->ringbuf+(nuHead)/*sizeof(short)*/;
            memcpy(pt,this->bufin,N*sizeof(short));
            nuHead += N;
            test = 1;
        }
        else
        {

            //first let us copy the part that fits
            int first = this->maxlength+1 - nuHead; //maxlength + 1 because of the additionnal value in the buffer
            pt = this->ringbuf+(nuHead)/*sizeof(short)*/;
            memcpy(pt,this->bufin,first*sizeof(short));
            //then what remains
            int second = N-first;
            pt = this->ringbuf;
            memcpy(pt,this->bufin+first/*sizeof(short)*/,second*sizeof(short));

            nuHead = second;
            test = 2;

        }
    }
    else
    {

        pt = this->ringbuf+(nuHead)/*sizeof(short)*/;
        memcpy(pt,this->bufin,N*sizeof(short));
        nuHead += N;
        test = 3;
    }


    if(nuHead > maxlength) qDebug()<<"err 3 "<<test;



    // ring_lock.lock();



    ring_lock.lock();
    this->head = nuHead;
    ring_lock.unlock();

    // if(this->freespace()<8000) {qDebug()<<"almost full :"<<this->freespace();}

}


int capture_port_c::pullN(unsigned long N)
{

    short *pt ;


    ring_lock.lock();


    unsigned long nuTail = this->tail;
    unsigned long nuHead = this->head;
    unsigned long length = this->length();
    ring_lock.unlock();





    if(length == 0) {return 0;}




    if(N > length)
        N = length;




    if((nuHead < nuTail)&&(nuTail+N>this->maxlength))
    {
        //first let us copy the part at the end of the ringbuffer
        unsigned long first = this->maxlength+1 - nuTail; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->ringbuf+(nuTail)/*sizeof(short)*/;
        memcpy(this->buffile,pt,first*sizeof(short));

        //then what remains
        unsigned long second = N-first;
        pt = this->ringbuf;
        memcpy(this->buffile+first/*sizeof(short)*/,pt,second*sizeof(short));

        nuTail = second;

    }
    else
    {
        pt = this->ringbuf+(nuTail)/*sizeof(short)*/;
        memcpy(this->buffile,pt,N*sizeof(short));
        nuTail += N;

    }


    //ring_lock.lock();

    ring_lock.lock();
    this->tail = nuTail;
    ring_lock.unlock();


    return N;
}


void capture_port_c::freeN(unsigned long N)
{

    if(length()<N) N = length();

    this->tail+=(N);

    while(this->tail>maxlength) this->tail-= maxlength;


}

void capture_port_c::destroyport()
{
    consumer->quit();
    delete this;
}

void capture_port_c::openfile(QString filename)
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

void capture_port_c::closefile()
{
    int err;
    if ((err = sf_close(soundfile))) {
        qDebug()<<"cannot close sndfile  for output error# "<< err;

        exit (1);
    }

    QDir r;



    QString old = filedir;
    filedir.replace(filedir.length()-3,3,"wav");


    r.rename(old,filedir);

}

void capture_port_c::startrecord(QString filename)
{

    //qDebug()<<"start record "<<id;


    if(recording)
    {
        qDebug()<<"already recording";
        return;
    }



    ring_lock.lock();
    if(freespace()<2000)
    {

        freeN(2000);
    }
    // empty();
    ring_lock.unlock();

    this->openfile(filename);

    recording = true;
    consumer->consumerLock = false;
    consumer->update();






    //qDebug()<<id<<"-start record 2";

}

void capture_port_c::stoprecord()
{
    //qDebug()<<"stop record "<<id;
    if(!recording) return;


    recording = false;

    this->closefile();




}




QElapsedTimer t;

void Consumer::update(void)
{
    int t1;
    static int tmax = 0;
    int nread,err;


    if(!port->recording)
        return;


    if(consumerLock) {return;}

    consumerLock = true;


    nread = port->pullN(NFILE_CAPTURE);



    t.start();
    if(nread >0 )
    {

        if ((err = sf_writef_short (port->soundfile, port->buffile,  nread) ) != nread)   qDebug()<< "cannot write sndfile"<<err;

    }
    t1 = t.elapsed();
    if(t1 > tmax) tmax = t1;

    if (t1 > 10) qDebug()<<"save delay: "<<t1<<"ms max : "<<tmax<<" ms";


    consumerLock = false;

}


void Consumer::run()
{



    while(1)
    {

        update();
        QThread::usleep(CAPTURE_WRITEFILE_SLEEP);
    }


}


Consumer::~Consumer()
{
    timer->stop();


}

Consumer::Consumer(void)
{

    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

}
