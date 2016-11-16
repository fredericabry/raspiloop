#ifndef LOOP_C_H
#define LOOP_C_H


#include <sndfile.h>
#include "playback_port_c.h"
#include "QThread"

#define STATUS_IDLE 0
#define STATUS_PLAY 1

class playback_loop_c;

class playbackLoopConsumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

public:
    playback_loop_c* controler;
    bool loopActive;
private slots:
    void datarequest(int frames);
    void activate(void);

signals:
    void send_data(short *buf,int nread);
};

class playback_loop_c:public QObject
{
    Q_OBJECT

public:
    playback_loop_c(const int id, playback_port_c *pPort, long length, bool autoplay);
    ~playback_loop_c();

    const int id;
    QString filename;
    playback_port_c *pPort;
    int status;

    SNDFILE *soundfile;
    short *buffile;


    long frametoplay;
    bool stop;
    bool repeat;
    playbackLoopConsumer *consumer;

    playback_loop_c *pPrevLoop,*pNextLoop;

    void destroyloop(bool opened);
    void test(QString a);
    void openFile(void);
    void moveToPort(playback_port_c *pNuPort);

    void play();
    void pause();

public slots:
    void consumerKilled();



};

#endif // LOOP_C_H
