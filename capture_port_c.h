#ifndef CAPTURE_PORT_C_H
#define CAPTURE_PORT_C_H



#include <sndfile.h>
#include <alsa/asoundlib.h>
#include <QMainWindow>
#include <qthread.h>

class capture_port_c;



class Consumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;
public:
    capture_port_c* port;

};





class capture_port_c : public QObject
{
    Q_OBJECT

public:
    capture_port_c(const unsigned long maxlength,const unsigned long bufsize,const int rate);
    ~capture_port_c();


    const unsigned long maxlength;
    const unsigned long bufsize;
    int const rate;
    SNDFILE *soundfile;

    bool recording;

    short *ringbuf;//large buffer for circular storage
    short *buf;//small buffer for transfert
    short *buffile;//small buffer for file writing

    unsigned long head;
    unsigned long tail;
    QThread *writeThread;
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

};


#endif // CAPTURE_PORT_C_H
