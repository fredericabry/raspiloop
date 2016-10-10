#include "ringbuf_c.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>
#include <sndfile.h>
#include "ringbuf_c.h"

#include <QFile>
#include <QTextStream>

#include <qdebug.h>


ringbuf_c::ringbuf_c(const int maxlength, const int bufsize, const int trigger):maxlength(maxlength),bufsize(bufsize),trigger(trigger)
{

    this->tail = 0;
    this->head = 0;

    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));
    buffile = (short*)malloc(10000*sizeof(short));

    soundfile = NULL;
}

ringbuf_c::~ringbuf_c()
{

    free(ringbuf);
    free(buf);
    free(buffile);
}

int ringbuf_c::push(short data)
{
    int next = this->head+1;
    if(next>= this->maxlength+1)
        next = 0;

    if(next == this->tail) //buffer is full
        return -1;


    this->ringbuf[this->head] = data;//inserting new data

    this->head = next;

    return 0;

}

int ringbuf_c::pull(short *data)
{

    if(this->head == this->tail)
    {
        triggerempty();
        return -1; //buffer is empty
    }
    *data = this->ringbuf[this->tail];

    int next = this->tail + 1;

    if(next >= this->maxlength+1) next = 0;

    this->tail = next;

    if(this->length()<trigger)
        triggerempty();
    return 0;

}

int ringbuf_c::length()
{
    if(this->head>=this->tail) return this->head-this->tail;
    else return this->maxlength-this->tail + this->head + 1;

}

int ringbuf_c::freespace()
{

    return this->maxlength-this->length();

}

void ringbuf_c::pushN(short *buf_in, int N)
{
    short *pt ;




    if(N > this->freespace())  {/* qDebug()<<"Insuficient free space in ringbuf struct";*/return;}

    if(N > this->maxlength) {qDebug()<<"Failed to copy to ringbuf struct"; return;}

    if(this->head >= this->tail)
    {
        if(N+this->tail <= this->maxlength)
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

int ringbuf_c::pullN(int N,short *buf0)
{

    short *pt ;
    int N0=0;
    if(N > this->length()) {
        /*qDebug()<<"not enough elements"; */
        N0 = N - this->length();
        N = this->length();
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
        //not enough elements in the ringbuffer, lets fill in with elements from buf0
        memcpy(this->buf+N/*sizeof(short)*/,buf0,N0*sizeof(short));

    }

    if(this->length()<trigger)
    {
        triggerempty();

    }

    return N+N0;
}


void ringbuf_c::triggerempty(void)
{
  //qDebug()<<"vide";
    soundupd();


}


void ringbuf_c::soundupd()
{
    int nread;



    if(!soundfile) {return; }


    if((nread = sf_readf_short(soundfile,buffile,10000))>0)
    {


       this->pushN(buffile,nread);

    }

    if(nread == 0)
    {
        sf_close(soundfile);
        this->soundfile = NULL;



    }


}
