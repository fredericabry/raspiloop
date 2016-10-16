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



capture_port_c::capture_port_c(const unsigned long maxlength, const unsigned long bufsize, const int rate):maxlength(maxlength),bufsize(bufsize),rate(rate)
{
    this->head = 0;
    this->tail = 0;



    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    memset(ringbuf,0,maxlength*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));
    memset(buf,0,bufsize*sizeof(short));
    buffile = (short*)malloc((NFILE_CAPTURE)*sizeof(short));
    memset(buffile,0,NFILE_CAPTURE*sizeof(short));
    recording = false;


}

capture_port_c::~capture_port_c()
{
    free(ringbuf);
    free(buf);
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
    short *pt ;

    unsigned long freespace = this->freespace();



    if(N > freespace)  {
        if(recording)
        {
        qDebug()<<"capture buffer full";

        }


       // qDebug()<<"capture ringbuf full";
        //not enough space, we need to forget N-freespace samples

        this->freeN(N-freespace);





    }

    if(N > this->maxlength) {qDebug()<<"Failed to copy to capture ringbuf struct"; return;}

    if(this->head >= this->tail)
    {
        if(N+this->head <= this->maxlength)
        {
            pt = this->ringbuf+(this->head)/*sizeof(short)*/;
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
        memcpy(pt,this->buf,N*sizeof(short));
        this->head += N;
    }


}

int capture_port_c::pullN(unsigned long N)
{


    short *pt ;
    unsigned long N0=0;
    unsigned long length = this->length();
    if(N > length) {
       // qDebug()<<"not enough elements";
        N0 = N - length;
        N = length;
    }

    if((this->head < this->tail)&&(this->tail+N>this->maxlength))
    {
        //first let us copy the part that fits
        int first = this->maxlength+1 - this->tail; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->ringbuf+(this->tail)/*sizeof(short)*/;
        memcpy(this->buffile,pt,first*sizeof(short));

        //then what remains
        int second = N-first;
        pt = this->ringbuf;

        memcpy(this->buffile+first/*sizeof(short)*/,pt,second*sizeof(short));

        this->tail = second;






    }
    else
    {
        pt = this->ringbuf+(this->tail)/*sizeof(short)*/;
        memcpy(this->buffile,pt,N*sizeof(short));
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
    this->tail+=(N);

    while(this->tail>maxlength) this->tail-= maxlength;


}

void capture_port_c::destroyport()
{
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

    if ((soundfile = sf_open (filename.toStdString().c_str(), SFM_WRITE, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile \"%s\" for output (%s)\n", "test.wav", errstr);

        exit (1);
    }


}

void capture_port_c::closefile()
{
    int err;
    if ((err = sf_close(soundfile))) {
        qDebug()<<"cannot open sndfile  for output error# "<< err;

        exit (1);
    }

}

void capture_port_c::startrecord(QString filename)
{
    if(recording)
    {
        qDebug()<<"already recording";
        //recording = false;
        return;
    }
//qDebug()<<"start recording";

   // freeN(10000);//let's make some room
    recording = true;

    this->openfile(filename);


    consumer = new Consumer();
    consumer->port = this;
    consumer->start();


}

void capture_port_c::stoprecord()
{

    recording = false;
   //qDebug()<<"stop recording";
    this->closefile();
    consumer->quit();
}

void Consumer::run()
{
    int nread;

    while(1)
    {
    if(!port->recording) {this->exit(0);break;}


    if(port->length()<TRIGGER_CAPTURE) {goto end;}



        nread = port->pullN(NFILE_CAPTURE);
        if(nread >0 )
        {
            //debugf("capture write "+n2s(nread)+ " samples");
            if (sf_write_raw (port->soundfile, port->buffile, sizeof(short)* nread) != sizeof(short)* nread)   qDebug()<< "cannot write sndfile";

          //  if(nread > 256) qDebug()<<nread;

        }

        end:
        sleep(0.0002);
    }

}
