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
    playback_loop_c(const int id, std::vector<playback_port_c*>pPorts, long length, syncoptions syncMode, status_t status,interface_c *interface);
    ~playback_loop_c();

    const int id;

   std::vector<playback_port_c*>pPorts;
   std::vector<int>portsChannel;
   std::vector<unsigned long>tails;
   std::vector<unsigned long>framesCount;
   std::vector<bool>loopConnectedToPort; //loop has been connected to a port



    unsigned long length();
    unsigned long length(unsigned long tail2);
    syncoptions syncMode;
    status_t status;
    interface_c *interface;

    QString filename;

    bool loopReadyToStop;//all file data has been transfered by the consumer to the ringbuffer. When the latter is empty we can destroy the loop.
    SNDFILE *soundfile;
    bool isFinished(void);



    bool addToPortList(playback_port_c* pNuPort);
    bool removeFromPortList(playback_port_c *pOldPort);


    short *ringbuf;//big buffer for circular storage

    short *buffile;//small buffer used to read files;

    unsigned long head;
    unsigned long maxlength;
    unsigned long bufsize;//size of "buf" used for transfert
    bool isFileOpened;


    long framestoplay;

    unsigned long min_frame_request;

    int barstoplay;
    bool stop;
    bool repeat;
    bool isOutOfSample;
    bool isClick;//is it used for the click ?
    playbackLoopConsumer *consumer;

    interfaceEvent_c *pEvent;

    playback_loop_c *pPrevLoop,*pNextLoop;
    QMutex playloop_mutex;
    int findCapturePortNumber(int channel);
    void test(QString a);
    void openFile(void);
    void moveToPort(playback_port_c *pNuPort);
    void rewind(void);
    bool isRewindRegistered;
    void registerRewind(void);//create the event to rewind the loop at the next appropriate moment
    void play();
    void pause();
    void updateFrameToPlay(long length);
    void addToList(void);
    void removeFromList(void);



    unsigned long freespace();
    void pushN(short *buf_in, unsigned long N);
    int pullN(unsigned long N);
    int pullN(unsigned long N, int portNumber, short **buf3);
    void destroy(void);

public slots:
    void activate(int channel);

private slots:
    void datarequest(unsigned long frames, int channel);
    void infoFromCaptureLoop(unsigned long length);
signals:
    void send_data(short *buf,int nread,int channel);
    void makeInterfaceEvent(const QObject * sender,const char * signal, int eventType, void *param,bool repeat,interfaceEvent_c** pEvent);




};

#endif // LOOP_C_H
