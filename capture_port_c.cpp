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
    ringbuf = (short*)malloc((maxlength+1)*sizeof(short));
    memset(ringbuf,0,maxlength*sizeof(short));
    bufin = (short*)malloc((bufsize)*sizeof(short));
    memset(bufin,0,bufsize*sizeof(short));
    ring_lock.unlock();
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
    ring_lock.unlock();

    if(N > this->maxlength) {qDebug()<<"Failed to copy to capture ringbuf struct";  return;}

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

    ring_lock.lock();
    this->head = nuHead;
    ring_lock.unlock();
}

void capture_port_c::destroyport()
{
    delete this;
}


