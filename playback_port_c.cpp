#include "playback_port_c.h"
#include "playback_loop_c.h"
#include <qdebug.h>
#include "parameters.h"
#include "interface.h"
#include "qtimer.h"

playback_port_c::playback_port_c(const unsigned long maxlength,  const int channel,  interface_c *interface):maxlength(maxlength),channel(channel),interface(interface)
{
    bufsize = PLAYBACK_BUFSIZE;


    this->tail = 0;
    this->head = 0;
    this->connected_loops = 0;
    this->nu_connected_loops = 0;

    consumer = new playbackPortConsumer;
    consumer->controler = this;

    //qDebug()<<maxlength;

    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));
    bufmix = (short*)malloc(bufsize*sizeof(short));

    wait_for_data = false;
    data_received = 0;

    consumer->start();

    interface->Afficher("playback port created");



}

playback_port_c::~playback_port_c()
{

    consumer->quit();
    free(ringbuf);
    free(buf);
    free(bufmix);

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



    if(N > this->freespace())  {qDebug()<<"playback ringbuf full"<<N;ring_lock.unlock();return;}

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

        if(!fg_empty)
        {

            //qDebug()<<channel<<"zeros";
            fg_empty=true;
        }


        memset(this->buf+N/*sizeof(short)*/,0,N0*sizeof(short));
    }else fg_empty = false;



    //we need to refill the ringbuffer
    //triggerempty();


    ring_lock.unlock();

    return N+N0;
}

void playback_port_c::addloop(playback_loop_c *pLoop)
{

    //this->connected_loops++;
    this->nu_connected_loops++;


   // qDebug()<<"nu connected"<<nu_connected_loops;

    if(nu_connected_loops <= 0) {qDebug()<<"err";return;}

    if(!connect(this,SIGNAL(signal_trigger(unsigned long,int)), pLoop, SLOT(datarequest(unsigned long,int)),Qt::UniqueConnection))
        qDebug()<<"connection failed";


    if(!connect(consumer,SIGNAL(update_loops()), pLoop, SLOT(activate()),Qt::UniqueConnection))
        qDebug()<<"connection failed";

    if( !connect(pLoop,SIGNAL(send_data(short*,int,int)), consumer, SLOT(data_available(short*, int,int)),Qt::UniqueConnection))
        qDebug()<<"connection failed";


    if(nu_connected_loops == 1)
    {
        pLoop->loopConnected = true;//first loop : let's avoid the activation procedure
        this->wait_for_data = false;//unblock the port

    }
//qDebug()<<"loop"<<pLoop->id<<"connected to"<<channel;



}

void playback_port_c::test(void)
{

    qDebug()<<"playback port ok "<<this->channel;
}

void playback_port_c::removeloop(playback_loop_c *pLoop)
{


    //this->connected_loops--;
    this->nu_connected_loops--;

    if(!disconnect(this,SIGNAL(signal_trigger(unsigned long,int)), pLoop, SLOT(datarequest(unsigned long,int))))
        qDebug()<<"playback loop disconnection failed 1";
    if( !disconnect(pLoop,SIGNAL(send_data(short*,int,int)), consumer, SLOT(data_available(short*, int,int))))
        qDebug()<<"playback loop  disconnection failed 2";
    if(!disconnect(consumer,SIGNAL(update_loops()), pLoop, SLOT(activate())))
        qDebug()<<"connection failed";





}

void playbackPortConsumer::run()
{
    datalength=0;
    QTimer updateTimer;
    connect(&updateTimer,SIGNAL(timeout()),this,SLOT(update()));
    updateTimer.start(10);
    exec();
}

void playback_port_c::triggerempty(void)
{

    unsigned long frames = this->freespace();


    if(wait_for_data)
    {
        //qDebug()<<"WAIT";
        return;
    }



    if(connected_loops != nu_connected_loops)
    {
        connected_loops = nu_connected_loops;//maybe we need to update the connect loops count
        emit consumer->update_loops(); //tell the added loops to get ready
    }


    if(connected_loops==0) {return;} //still zero ? return




    wait_for_data = true;
    // qDebug()<<"emit"<<frames;
    emit signal_trigger(frames,channel);



}

void playbackPortConsumer::data_available(short *buf, int nread,int chan)
{

//qDebug()<<"received";
    //implement mix strategy here
   // qDebug()<<chan<<controler->channel;
    if(chan != controler->channel) return;//not the right playback port




    if(nread > datalength) datalength = nread; //keep the longest sample size

    controler->data_received++;

    // qDebug()<<"received"<<controler->data_received<<nread;

    if(controler->connected_loops == 0) {
        qDebug()<<"bug no loop connected";
        controler->wait_for_data = false;
        datalength = 0;
        return;
    } //should not happen but let's not divide by 0






    if(controler->data_received == 1)//first loop
    {

        memset(controler->bufmix,0,controler->bufsize*sizeof(short));



        for(int i = 0;i<nread;i++)
        {

            controler->bufmix[i]=buf[i]/controler->connected_loops;
        }
    }
    else
    {

        for(int i = 0;i<nread;i++)
        {

            controler-> bufmix[i]+=buf[i]/controler->connected_loops;
        }
    }

    if(nread>0)
    delete buf;

    if(controler->data_received >= controler->connected_loops)
    {
        if(controler->data_received > controler->connected_loops) qDebug()<<"bug data received";



        controler->data_received = 0;
        //all data has been received, let's push it to the ringbuffer
        controler->pushN(controler->bufmix,datalength);
        datalength=0;
        controler->wait_for_data = false;


    }



}

void playbackPortConsumer::update()
{
    controler->triggerempty();

}

