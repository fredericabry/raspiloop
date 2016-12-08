#include "playback_port_c.h"
#include "playback_loop_c.h"
#include <qdebug.h>
#include "parameters.h"
#include "interface.h"
#include "qtimer.h"

playback_port_c::playback_port_c(const unsigned long maxlength,  const int channel,QString deviceName,interface_c *interface):channel(channel),deviceName(deviceName),interface(interface)
{



    consumer = new playbackPortConsumer(maxlength,PLAYBACK_BUFSIZE);
    consumer->controler = this;

    consumer->start();
    interface->Afficher("playback port "+QString::number(channel)+" created on "+deviceName);



}

void playback_port_c::addloop(playback_loop_c *pLoop)
{

    consumer->loop_lock.lock();

    if(std::find(pConnectedLoops.begin(), pConnectedLoops.end(), pLoop) != pConnectedLoops.end()) {
        //loop already connected
        qDebug()<<"error: loop already connected to port";
        consumer->loop_lock.unlock();
        return;
    }


    pConnectedLoops.push_back(pLoop);


    if((pConnectedLoops.size())==1)
        consumer->wait_for_data = false;//unblock the port

    consumer->loop_lock.unlock();

}

void playback_port_c::removeloop(playback_loop_c *pLoop)
{
    consumer->loop_lock.lock();
    pConnectedLoops.erase(std::remove(pConnectedLoops.begin(), pConnectedLoops.end(), pLoop), pConnectedLoops.end());
    consumer->loop_lock.unlock();
}


QString playback_port_c::getDeviceName(void)
{
    return deviceName;
}

playback_port_c::~playback_port_c()
{

    consumer->quit();


}

unsigned long playbackPortConsumer::length()
{
    if(this->head>=this->tail) return this->head-this->tail;
    else return maxlength-this->tail + this->head + 1;

}

unsigned long playbackPortConsumer::freespace()
{

    return maxlength-this->length();

}

void playbackPortConsumer::pushN(short *buf_in, unsigned long N)
{
    short *pt ;
    ring_lock.lock();



    if(N > this->freespace())  {ring_lock.unlock();return;}

    if(N > maxlength) {qDebug()<<"Failed to copy to ringbuf struct";ring_lock.unlock(); return;}

    if(this->head >= this->tail)
    {
        if(N+this->head <= maxlength)
        {
            pt = ringbuf+(this->head)/*sizeof(short)*/;
            memcpy(pt,buf_in,N*sizeof(short));
            this->head += N;
        }
        else
        {

            //first let us copy the part that fits
            int first = maxlength+1 - this->head; //maxlength + 1 because of the additionnal value in the buffer
            pt = ringbuf+(this->head)/*sizeof(short)*/;
            memcpy(pt,buf_in,first*sizeof(short));
            //then what remains
            int second = N-first;
            pt = ringbuf;
            memcpy(pt,buf_in+first/*sizeof(short)*/,second*sizeof(short));

            this->head = second;

        }
    }
    else
    {

        pt = ringbuf+(this->head)/*sizeof(short)*/;
        memcpy(pt,buf_in,N*sizeof(short));
        this->head += N;
    }
    ring_lock.unlock();

}

int playbackPortConsumer::pullN(unsigned long N)
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



    if((this->head < this->tail)&&(this->tail+N>maxlength))
    {
        //first let us copy the part that fits
        int first = maxlength+1 - this->tail; //maxlength + 1 because of the additionnal value in the buffer
        pt = ringbuf+(this->tail)/*sizeof(short)*/;
        memcpy(buf,pt,first*sizeof(short));

        //then what remains
        int second = N-first;
        pt = ringbuf;

        memcpy(buf+first/*sizeof(short)*/,pt,second*sizeof(short));

        this->tail = second;





    }
    else
    {

        pt = ringbuf+(this->tail)/*sizeof(short)*/;
        memcpy(buf,pt,N*sizeof(short));
        this->tail += N;

    }



    if(N0>0)
    {
        //not enough elements let's fill in with zeros

        if(!controler->fg_empty)
        {

            //qDebug()<<channel<<"zeros";
            controler->fg_empty=true;
        }


        memset(buf+N/*sizeof(short)*/,0,N0*sizeof(short));
    }else controler->fg_empty = false;






    ring_lock.unlock();

    return N+N0;
}

playbackPortConsumer::playbackPortConsumer(const unsigned maxlength, const unsigned long bufsize):maxlength(maxlength),bufsize(bufsize)
{
    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    bufmix = (short*)malloc(bufsize*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));

}


void playbackPortConsumer::run()
{


    wait_for_data = false;
    tail = 0;
    head = 0;


    QTimer updateTimer;
    connect(&updateTimer,SIGNAL(timeout()),this,SLOT(update()));
    updateTimer.start(10);

    exec();
}

void playbackPortConsumer::update()
{
    loop_lock.lock();
    int nread;
    short connected_loops;//nbr of loops connected to this ringbuffer

    unsigned long frames = freespace();


    if((wait_for_data))
    {
        qDebug()<<"WAIT";
        loop_lock.unlock();
        return;
    }


    connected_loops = controler->pConnectedLoops.size();


    if(connected_loops==0) { loop_lock.unlock();return;} //still zero ? return

    wait_for_data = true;


    memset(bufmix,0,bufsize*sizeof(short));

    short *buf2 = new short[PLAYBACK_BUFSIZE];

    datalength=0;


    //implement mix strategies here

    for (auto &pLoop : controler->pConnectedLoops)
    {
        memset(buf2,0,sizeof(short)*PLAYBACK_BUFSIZE);
        pLoop->datarequest(frames,controler->channel,buf2,&nread);
        if(nread > datalength) datalength = nread; //keep the longest sample size

        if(controler->interface->getMixStrategy() == AUTO)
        for(int i = 0;i<nread;i++)  bufmix[i]+=buf2[i]/connected_loops;
        else if(controler->interface->getMixStrategy() == PRESET)
        for(int i = 0;i<nread;i++)  bufmix[i]+=buf2[i]/(short)controler->interface->getMixLoopNumber();
    }

    pushN(bufmix,datalength);
    datalength=0;
    wait_for_data = false;
    delete buf2;

    loop_lock.unlock();

}

playbackPortConsumer::~playbackPortConsumer(void)
{
    free(ringbuf);
    free(bufmix);
    free(buf);
}
