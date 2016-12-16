#ifndef LOOP_C_H
#define LOOP_C_H


#include <sndfile.h>
#include "playback_port_c.h"
#include "QThread"
#include "qmutex.h"
#include "interface.h"
#include "qelapsedtimer.h"



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
    bool consumerReady;
protected:
    void destroyloop(void);
};



class playback_loop_c:public QObject
{
    Q_OBJECT

public:
    playback_loop_c(const int id, std::vector<playback_port_c*>pPorts, long length, syncoptions syncMode, status_t status, interface_c *interface, double delta);
    ~playback_loop_c();

    const int id;

   std::vector<playback_port_c*>pPorts;//ports to which the playloop is connected
   std::vector<int>portsChannel;//ports channel numer of the ports to which the playloop is connected
   std::vector<unsigned long>tails;//respective tail of each connected port used to have specific reading in the ringbuf
   std::vector<unsigned long>framesCount;//rames played on each port


    unsigned long length(unsigned long tail2);//return the length of the ringbuf for each port providing the right tail
    syncoptions syncMode;//synchronization mode of the loop [useful ? could we directly use the syncmode reading from the interface]
    status_t status;//playback status (playing/stoped/etc.)
    interface_c *interface;//pointer to the main interface

    QString filename;

    bool fileReadingOver;//all file data has been transfered by the consumer to the ringbuffer. When the latter is empty we can destroy the loop.
    SNDFILE *soundfile;
    bool isFinished(void);


    short *ringbuf;//big buffer for circular storage
    short *buffile;//small buffer used to read files;
    unsigned long head;
    unsigned long maxlength;
    unsigned long bufsize;//size of "buf" used for transfert
    bool isFileOpened;
    long framestoplay;
    unsigned long framestoskip;//if the loop was created a bit too late, we might want to skip some frames to stay with the click

    int barstoplay;
    bool stop;
    bool repeat;
    bool isOutOfSample;

    playbackLoopConsumer *consumer;

    interfaceEvent_c *pEvent;

    playback_loop_c *pPrevLoop,*pNextLoop;
    QMutex playloop_mutex;
    int findCapturePortNumber(int channel);

    void openFile(void);
    void moveToPort(std::vector<playback_port_c *> pNuPorts);


    void rewind(void);


    void play();
    void pause();
    void updateFrameToPlay(long length);
    void addToList(void);
    void removeFromList(void);
    void consumerReady();

    unsigned long freespace();
    void pushN(short *buf_in, unsigned long N);

    int pullN(unsigned long N, int portNumber, short **buf3);
    void destroy(void);


    void datarequest(unsigned long frames, int channel, short *buf2, int *nread);

signals:
    void send_data(short *buf,int nread,int channel);
    void makeInterfaceEvent(const QObject * sender,const char * signal, int eventType, void *param,bool repeat,interfaceEvent_c** pEvent);




};

#endif // LOOP_C_H
