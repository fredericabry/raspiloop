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

    short *ringbuf;//large buffer for circular storage
    short *bufin;//small buffer for transfert to the ringbuffer

    unsigned long head;

    QMutex ring_lock;

    void pushN(unsigned long N);
    void openfile(QString filename);
    void closefile();
    void destroyport();


};


#endif // CAPTURE_PORT_C_H
