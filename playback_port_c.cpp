#include "playback_port_c.h"
#include "playback_loop_c.h"
#include <qdebug.h>
#include "parameters.h"
#include "interface.h"
#include "qtimer.h"
#include "qthread.h"
#include "QElapsedTimer"

playback_port_c::playback_port_c(const unsigned long maxlength,  const int channel,QString deviceName,interface_c *interface):channel(channel),deviceName(deviceName),interface(interface)
{



    consumer = new playbackPortConsumer(maxlength,PLAYBACK_BUFSIZE);
    consumer->controler = this;
    consumer->running = true;
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


    // if(pLoop->isClick) qDebug()<<"click added";

    consumer->loop_lock.unlock();

}

void playback_port_c::removeloop(playback_loop_c *pLoop)
{
    consumer->loop_lock.lock();
    pConnectedLoops.erase(std::remove(pConnectedLoops.begin(), pConnectedLoops.end(), pLoop), pConnectedLoops.end());

    //if(pLoop->isClick) qDebug()<<"click removed";


    consumer->loop_lock.unlock();
}

void playback_port_c::destroy()
{
    consumer->quit();
    consumer->running = false;
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
      //  qDebug()<<controler->channel<<"not enough elements";
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

              //qDebug()<<controler->channel<<N0<<"zeros";
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



    tail = 0;
    head = 0;



    /*while(running)
    {
        update();
        QThread::usleep(PLAYBACK_MIX_SLEEP);
    }*/

    connect(controler->interface->timerMix,SIGNAL(timeout()),this,SLOT(update()));
    exec();

    controler->deleteLater();
}

void playbackPortConsumer::update()
{
    loop_lock.lock();
    controler->mixOver = false;
    int nread;
    short connected_loops;//nbr of loops connected to this ringbuffer

    unsigned long frames = freespace();


    connected_loops = controler->pConnectedLoops.size();

    if(connected_loops==0) { loop_lock.unlock();controler->mixOver = true;return;} //still zero ? return




    memset(bufmix,0,bufsize*sizeof(short));

    short *buf2 = new short[PLAYBACK_BUFSIZE];

    datalength=0;


    QElapsedTimer telaps;
    telaps.start();


    //implement mix strategies here

    for (auto &pLoop : controler->pConnectedLoops)
    {


        pLoop->datarequest(frames,controler->channel,buf2,&nread);

        if(nread > datalength) datalength = nread; //keep the longest sample size

        if(controler->interface->getAllMute())
        {
            for(int i = 0;i<nread;i++)
                bufmix[i]=0;
        }
        else
        {

            if(controler->interface->getMixStrategy() == AUTO)
                for(int i = 0;i<nread;i++)
                    bufmix[i]+=buf2[i]/connected_loops;
            else if(controler->interface->getMixStrategy() == PRESET)
                for(int i = 0;i<nread;i++)
                    bufmix[i]+=buf2[i]/(short)controler->interface->getMixLoopNumber();

        }
    }



    //sync all playports here

    controler->mixOver = true;

  // while(!controler->interface->isMixOver()){}

   // qDebug()<<controler->channel<<datalength<<telaps.nsecsElapsed()/1000;*/

    pushN(bufmix,datalength);




    delete buf2;



    loop_lock.unlock();





}

playbackPortConsumer::~playbackPortConsumer(void)
{
    free(ringbuf);
    free(bufmix);
    free(buf);
}
