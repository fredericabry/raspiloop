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

#include <qdebug.h>
#include <qelapsedtimer.h>
#include "qthreadpool.h"


capture_port_c::capture_port_c(const unsigned long maxlength, const unsigned long bufsize, const int rate, const int id):maxlength(maxlength),bufsize(bufsize),rate(rate),id(id)
{
    this->head = 0;
    this->tail = 0;

    ring_lock.lock();

    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    memset(ringbuf,0,maxlength*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));
    memset(buf,0,bufsize*sizeof(short));
    buffile = (short*)malloc((NFILE_CAPTURE)*sizeof(short));
    memset(buffile,0,NFILE_CAPTURE*sizeof(short));
    recording = false;

    ring_lock.unlock();


    consumer = new Consumer();
    consumer->port = this;

    consumer->start();



}

capture_port_c::~capture_port_c()
{
    ring_lock.lock();
    consumer->quit();
    free(ringbuf);
    free(buf);
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

void capture_port_c::empty(void)
{

    ring_lock.lock();
    tail = head-1;
    ring_lock.unlock();

}

void capture_port_c::pushN(unsigned long N)
{
    static bool fg_full = false;

    ring_lock.lock();



    short *pt ;

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


        // qDebug()<<"capture ringbuf full";
        //not enough space, we need to forget N-freespace samples

        this->freeN(N-freespace);

    }else fg_full = false;





    if(N > this->maxlength) {qDebug()<<"Failed to copy to capture ringbuf struct"; ring_lock.unlock(); return;}

    if(this->head >= this->tail)
    {
        if(N+this->head <= this->maxlength)
        {
            pt = this->ringbuf+(this->head)/*sizeof(short)*/;
            //    qDebug()<<"5"<<" ; "<<this->tail<<" ; "<<this->head<<" ; "<<length()<<" ; "<<N;
            memcpy(pt,this->buf,N*sizeof(short));
            this->head += N;
        }
        else
        {

            //first let us copy the part that fits
            int first = this->maxlength+1 - this->head; //maxlength + 1 because of the additionnal value in the buffer
            pt = this->ringbuf+(this->head)/*sizeof(short)*/;
            memcpy(pt,this->buf,first*sizeof(short));
            //then what remains
            int second = N-first;
            pt = this->ringbuf;
            memcpy(pt,this->buf+first/*sizeof(short)*/,second*sizeof(short));

            this->head = second;

        }
    }
    else
    {

        pt = this->ringbuf+(this->head)/*sizeof(short)*/;
        //qDebug()<<"4"<<" ; "<<this->tail<<" ; "<<this->head<<" ; "<<length()<<" ; "<<N;
        memcpy(pt,this->buf,N*sizeof(short));
        this->head += N;
    }


    if(head > maxlength) qDebug()<<"err 3";


    ring_lock.unlock();
}

int capture_port_c::pullN(unsigned long N)
{





    short *pt ;
    unsigned long N0=0;
    unsigned long length = this->length();


    if(N > length) {
        //qDebug()<<"not enough elements";
        N0 = N - length;
        N = length;
    }

    if((this->head < this->tail)&&(this->tail+N>this->maxlength))
    {
        //first let us copy the part that fits
        int first = this->maxlength+1 - this->tail; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->ringbuf+(this->tail)/*sizeof(short)*/;

        //qDebug()<<"1"<<" ; "<<this->tail<<" ; "<<this->head<<" ; "<<length<<" ; "<<N;
        memcpy(this->buffile,pt,first*sizeof(short));

        //then what remains
        int second = N-first;
        pt = this->ringbuf;
        //qDebug()<<"2"<<" ; "<<this->tail<<" ; "<<this->head<<" ; "<<length<<" ; "<<N;
        memcpy(this->buffile+first/*sizeof(short)*/,pt,second*sizeof(short));

        this->tail = second;

        if(tail > maxlength) qDebug()<<"err 1";




    }
    else
    {
        pt = this->ringbuf+(this->tail)/*sizeof(short)*/;
        //qDebug()<<"3"<<" ; "<<this->tail<<" ; "<<this->head<<" ; "<<length<<" ; "<<N;
        memcpy(this->buffile,pt,N*sizeof(short));

        if(tail+N > maxlength) qDebug()<<"err 2"<<" ; "<<this->tail<<" ; "<<this->head<<" ; "<<length<<" ; "<<this->length()<<" ; "<<N;

        this->tail += N;



    }



    if(N0>0)
    {
        //not enough elements let's fill in with zeros
        // memset(this->buffile+N/*sizeof(short)*/,0,N0*sizeof(short));
    }




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

    const char * fn = filename.toStdString().c_str();

    int cmpt = 0;
    while ((soundfile = sf_open (fn, SFM_WRITE, &sf_info)) == NULL) {
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

}

void capture_port_c::startrecord(QString filename)
{



    if(recording)
    {
        qDebug()<<"already recording";
        return;
    }



    ring_lock.lock();
    freeN(length()/2);
    ring_lock.unlock();

    this->openfile(filename);

    recording = true;
    consumer->update();






    //qDebug()<<id<<"-start record 2";

}

void capture_port_c::stoprecord()
{
    //qDebug()<<id<<"-stop record 1";
    if(!recording) return;


    recording = false;


   // consumer->quit();

    this->closefile();


}




void Consumer::updtimer(void)
{

    update();

}





void Consumer::update(void)
{

//qDebug()<<"tooop";


if(!port->recording) return;


int nread,err;




port->ring_lock.lock();



nread = port->pullN(NFILE_CAPTURE);



if(nread >0 )
{
    //  qDebug()<<port->id<<"-consumer";
    if ((err = sf_write_raw (port->soundfile, port->buffile, sizeof(short)* nread) ) != sizeof(short)* nread)   qDebug()<< "cannot write sndfile"<<err;

    //  if(nread > 256) qDebug()<<nread;

}
//qDebug()<<nread;

port->ring_lock.unlock();



}



void Consumer::run()
{

  /*  QTimer timer;
    timer.moveToThread(this);
    timer.start(10);

    connect(&timer,SIGNAL(timeout()), this, SLOT(updtimer()));
    exec();*/

    while(1)
    {

        update();
        QThread::usleep(CAPTURE_WRITEFILE_SLEEP);



    }






}
