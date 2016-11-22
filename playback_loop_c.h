#ifndef LOOP_C_H
#define LOOP_C_H


#include <sndfile.h>
#include "playback_port_c.h"
#include "QThread"
#include "qmutex.h"
#include "interface.h"






class playback_loop_c;

class playbackLoopConsumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

public:
    playback_loop_c* controler;
    bool update_lock ;
    bool active;//consumer active as long as this is true
    void update(void);
    void stop(void);
protected:
    void destroyloop(void);
};





class playback_loop_c:public QObject
{
    Q_OBJECT

public:
    playback_loop_c(const int id, playback_port_c *pPort, long length, syncoptions syncMode, status_t status);
    ~playback_loop_c();

    const int id;

    playback_port_c *pPort;

    unsigned long length();
    syncoptions syncMode;
    status_t status;


    QString filename;
    bool loopConnected;//loop has been connected to a port
    bool loopReadyToStop;//all file data has been transfered by the consumer to the ringbuffer. When the latter is empty we can destroy the loop.
    SNDFILE *soundfile;


    short *ringbuf;//big buffer for circular storage
    short *buf;//small buffer for transfert
    short *buffile;//small buffer used to read files;

    unsigned long tail,head;
    unsigned long maxlength;
    unsigned long bufsize;//size of "buf" used for transfert
    bool isFileOpened;


    long framestoplay;
    long framescount;


    int barstoplay;
    bool stop;
    bool repeat;
    bool isOutOfSample;
    bool isClick;//is it used for the click ?
    playbackLoopConsumer *consumer;

    interfaceEvent_c *pEvent;

    playback_loop_c *pPrevLoop,*pNextLoop;
    QMutex playloop_mutex;

    void test(QString a);
    void openFile(void);
    void moveToPort(playback_port_c *pNuPort);
    void rewind(void);
    bool isRewindRegistered;
    void registerRewind(void);//create the event to rewind the loop at the next appropriate moment
    void play();
    void pause();
    void updateFrameToPlay(long length);




    unsigned long freespace();
    void pushN(short *buf_in, unsigned long N);
    int pullN(unsigned long N);
    void destroy(void);


private slots:
    void datarequest(unsigned long frames);
    void activate(void);
signals:
    void send_data(short *buf,int nread);
    void makeInterfaceEvent(const QObject * sender,const char * signal, int eventType, void *param,bool repeat,playback_loop_c *pLoop);




};

#endif // LOOP_C_H
