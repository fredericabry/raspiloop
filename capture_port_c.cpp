#include "capture_port_c.h"
#include "qdebug.h"
#include "interface.h"




capture_port_c::capture_port_c(const unsigned long maxlength, const unsigned long bufsize, const int channel, const QString deviceName, interface_c *interface):maxlength(maxlength),channel(channel),deviceName(deviceName),interface(interface)
{
    ring_lock.lock();
    this->head = 0;
    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    memset(ringbuf,0,maxlength*sizeof(short));
    bufin = (short*)malloc((bufsize)*sizeof(short));
    memset(bufin,0,bufsize*sizeof(short));
    ring_lock.unlock();

    interface->Afficher("capture port "+QString::number(channel)+" created on "+deviceName);
}

capture_port_c::~capture_port_c()
{
    free(ringbuf);
    free(bufin);

}




void capture_port_c::pushN(unsigned long N)
{
    short *pt ;

    ring_lock.lock();
    unsigned long nuHead = this->head;
    //  ring_lock.unlock();

    if(N > this->maxlength) {qDebug()<<"Failed to copy to capture ringbuf struct";  ring_lock.unlock();return;}

    if(N+nuHead <= this->maxlength)
    {
        pt = this->ringbuf+(nuHead)/*sizeof(short)*/;
        memcpy(pt,this->bufin,N*sizeof(short));
        nuHead += N;
    }
    else
    {
        int first = this->maxlength+1 - nuHead; //maxlength + 1 because of the additionnal value in the buffer
        pt = this->ringbuf+(nuHead)/*sizeof(short)*/;
        memcpy(pt,this->bufin,first*sizeof(short));
        //then what remains
        int second = N-first;
        pt = this->ringbuf;
        memcpy(pt,this->bufin+first/*sizeof(short)*/,second*sizeof(short));
        nuHead = second;
    }



    // ring_lock.lock();
    this->head = nuHead;
    ring_lock.unlock();
}





