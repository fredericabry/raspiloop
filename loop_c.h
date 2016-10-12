#ifndef LOOP_C_H
#define LOOP_C_H


#include <sndfile.h>
#include <alsa/asoundlib.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>
#include "playback_port_c.h"


#define PLAYBACK_CHANNEL_WIDTH 256 //number of elements in a a frame for ONE channel
#define NFILE 1000
#define THRESHOLD 5000
#define RINGBUFSIZE 80000




class loop_c:public QObject
{
Q_OBJECT

public:
    loop_c();
    ~loop_c();


    SNDFILE *soundfile;
    short *buffile;
    playback_port_c *pRing;

    int frametoplay;
    bool stop;

    void init(QString file, playback_port_c *pRingBuffer, int length);
    void destroyloop(void);

    private slots:
    void datarequest(void);

    signals:
    void send_data(short *buf,int nread);

};

#endif // LOOP_C_H
