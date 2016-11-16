#ifndef PLAYBACK_PORT_C_H
#define PLAYBACK_PORT_C_H

#include <sndfile.h>
#include <qthread.h>
#include <qmutex.h>


class interface_c;
class playback_loop_c;
class playback_port_c;


class playbackPortConsumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

public:
    playback_port_c* controler;
    int datalength;
public slots:
    void data_available(short *buf,int nread);

signals:
    void update_loops(void);//signal all loop that they are now autorized to feed data

};




class playback_port_c : public QObject
{
    Q_OBJECT

public:
    playback_port_c(const unsigned long maxlength,const unsigned long bufsize, const unsigned long trigger, const int channel, interface_c *interface);
    ~playback_port_c();

   // const interface_c interface;
    const unsigned long maxlength;
    const unsigned long bufsize;
    const unsigned long trigger;
    int const channel;
    interface_c *interface;
    short *ringbuf;//big buffer for circular storage
    short *buf;//small buffer for transfert
    short *buffile;//small buffer used to read files;

    unsigned long head;
    unsigned long tail;

    int data_received;//number of buf sent to the ringbuf by the loops connected to it.
    bool wait_for_data;

    playbackPortConsumer *consumer;


    void removeallloops(void);

    int push(short data);
    int pull(short *data);
    void fill(int channel);
    unsigned long length();
    unsigned long freespace();
    void pushN(short *buf_in, unsigned long N);
    int pullN(unsigned long N);
    void triggerempty(void);
    void addloop(playback_loop_c *pLoop);
    void removeloop(playback_loop_c *pLoop);

   // loop_c **pLoops;
    int connected_loops;//nbr of loops connected to this ringbuffer
    int nu_connected_loops;//new value of the connect loop count, update of connected_loops is done AFTER all data is collected

    QMutex ring_lock;

signals:
    void signal_trigger(int frames);





};

#endif // OU_C_H





