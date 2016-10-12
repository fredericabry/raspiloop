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
    this->oldest = 0;




    ringbuf = (short*)malloc((maxlength)*sizeof(short));
    memset(ringbuf,0,maxlength*sizeof(short));
    buf = (short*)malloc((bufsize)*sizeof(short));
    memset(buf,0,bufsize*sizeof(short));
    recording = false;


}

capture_port_c::~capture_port_c()
{
    free(ringbuf);
    free(buf);
}

void capture_port_c::pushN(unsigned long N)
{
    short *dest;

    if(N>bufsize)
    {
        qDebug()<<"capture transfert buffer too small";
        return;

    }

    if(N>maxlength)
    {
        qDebug()<<"capture ring buffer too small";
        return;
    }


    dest = ringbuf + oldest; //position of the oldest sample in the ringbuffer

    if(oldest + N < maxlength)
    {
        memcpy(dest,this->buf,N*sizeof(short));
        oldest = oldest + N;

        return;
    }
    else
    {
        int N0 = maxlength - oldest;
        memcpy(dest,this->buf,N0*sizeof(short));
        memcpy(ringbuf,this->buf+N0,(N-N0)*sizeof(short));
        oldest = oldest + N - maxlength;

        return;
    }
}

void capture_port_c::pullN(unsigned long pos, unsigned long N)
{
    if((N>bufsize)||(pos>bufsize))
    {
        qDebug()<<"capture transfert buffer too small";
        return;

    }

    if((N>maxlength)||(pos>maxlength))
    {
        qDebug()<<"capture ring buffer too small";
        return;
    }


    if(pos+N<maxlength)
    {

        memcpy(buf,ringbuf+pos,N*sizeof(short));


    }
    else
    {
        int N0 = maxlength-pos;
        memcpy(buf,ringbuf+pos,N0*sizeof(short));
        memcpy(buf+N0,ringbuf,(N-N0)*sizeof(short));
    }

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

void capture_port_c::startrecord()
{


}

void capture_port_c::stoprecord()
{


}
