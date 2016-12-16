#ifndef CAPTURE_LOOP_C_H
#define CAPTURE_LOOP_C_H

#include <QMainWindow>
#include <sndfile.h>
#include "parameters.h"
#include "qstring.h"
#include "qthread.h"
#include "qelapsedtimer.h"
#include "capture_port_c.h"



class capture_loop_c;
class playback_port_c;
class playback_loop_c;
class click_c;
class interfaceEvent_c;

class captureLoopConsumer:public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE;
public:
    capture_loop_c* controler;
    QElapsedTimer telapsed;
public slots:
    void update(void);
};


class capture_loop_c:public QObject
{
    Q_OBJECT
public:
    capture_loop_c(const int id, capture_port_c *pPort, long length, bool createPlayLoop, std::vector<playback_port_c *> pPlayPort, double delta);
    ~capture_loop_c();
    void destroyLoop(void);
    void openfile(QString filename);
    void closefile(void);
    void preDestroy(void);
    int pullN(unsigned long N);

    void removeFromList(void);
    void addToList(void);
    unsigned long framesCount; //number of frames recorded so far
    unsigned int barCount;
    const int id;
    QString filename;
    capture_port_c *pPort;
    bool createPlayLoop;//auto create playloop when closing capture loop
    std::vector<playback_port_c *> pPlayPorts;//if playloop is created it will be on these play ports
    //used to keep track of every capture loops
    capture_loop_c *pNextLoop;
    capture_loop_c *pPrevLoop;
    SNDFILE *soundfile;//file handler
    short *buffile; //buffer used for file writting
    unsigned long tail;//last value read in the associated ringbuffer
    captureLoopConsumer *consumer;
    bool recording;
    QString filedir;
    bool stop;//true if a length is specified and therefore the recording has to self stop
    long framesToRead;//values left to be recorded

signals:
    void makeInterfaceEvent(const QObject * sender,const char * signal, int eventType, void *param,bool repeat,interfaceEvent_c** pEvent);



};

#endif // CAPTURE_LOOP_C_H





