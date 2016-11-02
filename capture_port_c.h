#ifndef CAPTURE_PORT_C_H
#define CAPTURE_PORT_C_H



#include <sndfile.h>
#include <alsa/asoundlib.h>
#include <QMainWindow>
#include <qthread.h>
#include <qmutex.h>
#include "qtimer.h"
#include "qrunnable.h"
#include "qelapsedtimer.h"

class capture_port_c;



class Consumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;
public:
    capture_port_c* port;

    bool consumerLock;
QElapsedTimer t;
    QTimer timer;

public slots:
        void update(void);


};







class capture_port_c : public QObject
{
    Q_OBJECT

public:
    capture_port_c(const unsigned long maxlength,const unsigned long bufsize,const int rate,const int id);
    ~capture_port_c();


    const unsigned long maxlength;
    const unsigned long bufsize;
    int const rate;
    int const id;
    SNDFILE *soundfile;
    QString filedir;

    bool recording;

    short *ringbuf;//large buffer for circular storage
    short *bufin;//small buffer for transfert to the ringbuffer
    short *buffile;//small buffer for file writing from the ringbuffer

    unsigned long head;
    unsigned long tail;

    unsigned long readStartPoint,readEndPoint;

    Consumer *consumer;


    void fill(int channel);
    void pushN(unsigned long N);
    int pullN(unsigned long N);
    void openfile(QString filename);
    void closefile();
    void destroyport();
    void startrecord(QString filename);
    void stoprecord();

    unsigned long  length();
    unsigned long  freespace();
    void freeN(unsigned long N);
    QMutex ring_lock;

};


#endif // CAPTURE_PORT_C_H
