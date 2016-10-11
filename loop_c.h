#ifndef LOOP_C_H
#define LOOP_C_H


#include <sndfile.h>
#include <alsa/asoundlib.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>
#include "ringbuf_c.h"


#define NFILE 500
#define THRESHOLD 1000
#define RINGBUFSIZE 50000

class loop_c:public QObject
{
Q_OBJECT

public:
    loop_c();
    ~loop_c();


    SNDFILE *soundfile;
    short *buffile;
    ringbuf_c *pRing;



    void init(QString file,ringbuf_c *pRingBuffer);
    void destroyloop(void);

    private slots:
    void datarequest(void);

    signals:
    void send_data(short *buf,int nread);

};

#endif // LOOP_C_H
