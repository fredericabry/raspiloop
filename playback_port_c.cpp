#include "playback_port_c.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>
#include <sndfile.h>
#include "playback_port_c.h"
#include <QMainWindow>
#include <QFile>
#include <QTextStream>

#include <qdebug.h>


playback_port_c::playback_port_c(const unsigned long maxlength, const unsigned long bufsize, const unsigned long trigger, const int channel):maxlength(maxlength),bufsize(bufsize),trigger(trigger),channel(channel)
{

    this->tail = 0;
    this->head = 0;
    this->connected_loops = 0;

    //qDebug()<<maxlength;

    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));
    buffile = (short*)malloc(NFILE*sizeof(short));


}

playback_port_c::~playback_port_c()
{

    free(ringbuf);
    free(buf);
    free(buffile);
}

int playback_port_c::push(short data)
{
    unsigned long next = this->head+1;
    if(next>= this->maxlength+1)
        next = 0;

    if(next == this->tail) //buffer is full
        return -1;


    this->ringbuf[this->head] = data;//inserting new data

    this->head = next;

    return 0;

}

int playback_port_c::pull(short *data)
{

    if(this->head == this->tail)
    {
        triggerempty();
        return -1; //buffer is empty
    }
    *data = this->ringbuf[this->tail];

    unsigned long next = this->tail + 1;

    if(next >= this->maxlength+1) next = 0;

    this->tail = next;

    if(this->length()<trigger)
        triggerempty();




    return 0;

}

unsigned long playback_port_c::length()
{
    if(this->head>=this->tail) return this->head-this->tail;
    else return this->maxlength-this->tail + this->head + 1;

}

unsigned long playback_port_c::freespace()
{

    return this->maxlength-this->length();

}

void playback_port_c::pushN(short *buf_in, unsigned long N)
{
    short *pt ;




    if(N > this->freespace())  { qDebug()<<"ringbuf full";return;}

    if(N > this->maxlength) {qDebug()<<"Failed to copy to ringbuf struct"; return;}

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


}

int playback_port_c::pullN(unsigned long N)
{

/*
    memset(buf,0,N*sizeof(short));
    return N;*/

    short *pt ;
    unsigned long N0=0;
    unsigned long length = this->length();
    if(N > length) {
        /*qDebug()<<"not enough elements"; */
        N0 = N - length;
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



    if(N0>0)
    {
        //not enough elements let's fill in with zeros
        memset(this->buf+N/*sizeof(short)*/,0,N0*sizeof(short));
    }

    if(length<trigger)
    {
        triggerempty();
       /* if (length<THRESHOLD_WARNING)
             qDebug()<<"almost full";*/
    }



    return N+N0;
}

void playback_port_c::triggerempty(void)
{

    data_received = 0;//reset the amount of data received
    emit signal_trigger();

}

void playback_port_c::addloop()
{
    this->connected_loops++;
    //qDebug()<<"new loop connection to channel #"<<channel<<" total connections "<< connected_loops;

}

void playback_port_c::removeloop()
{
    this->connected_loops--;
    //  qDebug()<<"loop disconnected "<< connected_loops;

}

void playback_port_c::data_available(short *buf, int nread)
{

    //implement mix strategy here



    data_received++;

    if(data_received == 1)//first loop
    {
        memset(buffile,0,NFILE*sizeof(short));
        for(int i = 0;i<nread;i++)
        {
            buffile[i]=buf[i]/connected_loops;
        }
    }
    else
    {
        for(int i = 0;i<nread;i++)
        {
            buffile[i]+=buf[i]/connected_loops;
        }
    }


    if(data_received >= this->connected_loops)
    {
        data_received = 0;
        //all data has been received, let's push it to the ringbuffer
        pushN(buffile,NFILE);

    }





}
