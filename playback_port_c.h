#ifndef PLAYBACK_PORT_C_H
#define PLAYBACK_PORT_C_H

#include <sndfile.h>
#include <qthread.h>
#include <qmutex.h>
#include "qelapsedtimer.h"

class interface_c;
class playback_loop_c;
class playback_port_c;


class playbackPortConsumer:public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE;
    ~playbackPortConsumer();

public:
    playbackPortConsumer(const unsigned maxlength,const unsigned long bufsize);
    playback_port_c* controler;
    int datalength;

    int pullN(unsigned long N);
    void pushN(short *buf_in, unsigned long N);
    unsigned long length();
    unsigned long freespace();
    QMutex ring_lock;
    QMutex loop_lock;
    short *buf;//small buffer for transfert
    bool running;


public slots:
    void update(void);

private:
    const unsigned long maxlength;
    unsigned long bufsize;
    unsigned long head;
    unsigned long tail;
    short *ringbuf;//big buffer for circular storage
    short *bufmix;//small buffer used to mix data from every loops;

};




class playback_port_c : public QObject
{
    Q_OBJECT

public:

    playback_port_c(unsigned long maxlength, const int channel,const QString deviceName, interface_c *interface);
    ~playback_port_c();



    int const channel;
    const QString deviceName;
    interface_c *interface;
    playbackPortConsumer *consumer;
    bool fg_empty ;
    void addloop(playback_loop_c *pLoop);
    void removeloop(playback_loop_c *pLoop);
    QString getDeviceName(void);
    std::vector<playback_loop_c*>pConnectedLoops;
    bool nuTop;
    QElapsedTimer telapsed;



    unsigned long clickDataToPlay0;
    unsigned long clickDataToPlay1;

};

#endif // OU_C_H





