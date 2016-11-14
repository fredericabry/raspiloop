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
#include "playback_loop_c.h"
#include <qdebug.h>
#include <qelapsedtimer.h>

playback_port_c::playback_port_c(const unsigned long maxlength, const unsigned long bufsize, const unsigned long trigger, const int channel):maxlength(maxlength),bufsize(bufsize),trigger(trigger),channel(channel)
{

    this->tail = 0;
    this->head = 0;
    this->connected_loops = 0;


    consumer = new playbackPortConsumer;
    consumer->controler = this;

    //qDebug()<<maxlength;

    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));
    buffile = (short*)malloc(NFILE_PLAYBACK*sizeof(short));

    wait_for_data = false;
    data_received = 0;

    consumer->start();



}

playback_port_c::~playback_port_c()
{

    consumer->quit();
    free(ringbuf);
    free(buf);
    free(buffile);
    //free(pLoops);
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
ring_lock.lock();



    if(N > this->freespace())  { qDebug()<<"playback ringbuf full"<<N;ring_lock.unlock();return;}

    if(N > this->maxlength) {qDebug()<<"Failed to copy to ringbuf struct";ring_lock.unlock(); return;}

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
ring_lock.unlock();

}

int playback_port_c::pullN(unsigned long N)
{

    ring_lock.lock();
    static bool fg_empty = false;



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

        if(!fg_empty) {fg_empty = true;/*qDebug()<<"zeros";*/fg_empty=true;}


        memset(this->buf+N/*sizeof(short)*/,0,N0*sizeof(short));
    }else fg_empty = false;

    if(length<trigger)
    {
        triggerempty();
        /* if (length<THRESHOLD_WARNING)
             qDebug()<<"almost full";*/
    }

ring_lock.unlock();

    return N+N0;
}

void playback_port_c::triggerempty(void)
{

   // data_received = 0;//reset the amount of data received
    if(connected_loops==0) {return;}


   if(wait_for_data)
   {

       return;
   }


   wait_for_data = true;

   emit signal_trigger(this->freespace());

 /*
   for(int i = 0;i<connected_loops;i++)
   {
    loop_c *loop = this->pLoops[i];

    loop->datarequest(this->freespace());


   }
*/




}

void playback_port_c::addloop(playback_loop_c *pLoop)
{

    this->connected_loops++;

    this->wait_for_data = false;

    if(connected_loops <= 0) {qDebug()<<"err";return;}

    /*
    if(connected_loops == 1)
    {

        this->pLoops = (loop_c**)malloc(sizeof(loop_c*));
        this->pLoops[0] = pLoop;


    }
    else
    {

        this->pLoops = (loop_c**)realloc(this->pLoops,connected_loops*sizeof(loop_c*));
        this->pLoops[connected_loops-1] = pLoop;

    }

*/



}

void playback_port_c::removeloop()
{
    this->connected_loops--;


    /*if (connected_loops >0)
    this->pLoops = (loop_c**)realloc(this->pLoops,connected_loops*sizeof(loop_c*));
*/


    //  qDebug()<<"loop disconnected "<< connected_loops;

}


void playbackPortConsumer::run()
{
    exec();
}


void playbackPortConsumer::data_available(short *buf, int nread)
{

    //implement mix strategy here
    controler->data_received++;


    if(controler->data_received == 1)//first loop
    {
        memset(controler->buffile,0,nread*sizeof(short));
        for(int i = 0;i<nread;i++)
        {
            controler->buffile[i]=buf[i]/controler->connected_loops;
        }
    }
    else
    {
        for(int i = 0;i<nread;i++)
        {
           controler-> buffile[i]+=buf[i]/controler->connected_loops;
        }
    }


    if(controler->data_received >= controler->connected_loops)
    {
        controler->data_received = 0;
        //all data has been received, let's push it to the ringbuffer
        controler->pushN(controler->buffile,nread);
        controler->wait_for_data = false;

    }





}


void playback_port_c::removeallloops(void)
{


  //for(int i = 0;i<connected_loops;i++) this->pLoops[i]->destroyloop();

}



