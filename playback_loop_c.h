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
#include "QThread"



class playback_loop_c;





class playbackLoopConsumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

public:
    playback_loop_c* controler;


private slots:
    void datarequest(int frames);

signals:
    void send_data(short *buf,int nread);
};










class playback_loop_c:public QObject
{
    Q_OBJECT

public:
    playback_loop_c(const QString id,const QString filename, playback_port_c *pRing, int length);
    ~playback_loop_c();

    const QString id;
    const QString filename;
    playback_port_c *pRing;

    SNDFILE *soundfile;
    short *buffile;


    int frametoplay;
    bool stop;
    bool repeat;
    playbackLoopConsumer *consumer;


    void destroyloop(bool opened);
    void test(QString a);




};

#endif // LOOP_C_H
