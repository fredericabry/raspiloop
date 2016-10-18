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


class loop_c:public QObject
{
Q_OBJECT

public:
    loop_c(const QString id,const QString filename, playback_port_c *pRing, int length);
    ~loop_c();

    const QString id;
    const QString filename;
    playback_port_c *pRing;

    SNDFILE *soundfile;
    short *buffile;


    int frametoplay;
    bool stop;
    bool repeat;



    void destroyloop(void);
    void test(QString a);


    private slots:
    void datarequest(int frames);

    signals:
    void send_data(short *buf,int nread);

};

#endif // LOOP_C_H
