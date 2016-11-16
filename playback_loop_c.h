#ifndef LOOP_C_H
#define LOOP_C_H


#include <sndfile.h>
#include <stdbool.h>
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



private slots:
    void datarequest(int frames);

signals:
    void send_data(short *buf,int nread);
};










class playback_loop_c:public QObject
{
    Q_OBJECT

public:
    playback_loop_c(const QString id,const QString filename, playback_port_c *pPort, long length,playback_loop_c *pPrevLoop);
    ~playback_loop_c();

    const QString id;
    const QString filename;
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

    void play();
    void pause();

public slots:
    void consumerKilled();



};

#endif // LOOP_C_H
