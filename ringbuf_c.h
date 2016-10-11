#ifndef RINGBUF_C_H
#define RINGBUF_C_H

#include <sndfile.h>
#include <alsa/asoundlib.h>
#include <QMainWindow>


class ringbuf_c : public QObject
{
    Q_OBJECT

public:
    ringbuf_c(const int maxlength,const int bufsize, const int trigger);
    ~ringbuf_c();


    int const maxlength;
    int const bufsize;
    const int trigger;

    short *ringbuf;//big buffer for circular storage
    short *buf;//small buffer for transfert
    short *buffile;//small buffer used to read files;

    int head;
    int tail;

    int data_received;//number of buf sent to the ringbuf by the loops connected to it.


    SNDFILE *soundfile;


    int push(short data);
    int pull(short *data);
    void fill(int channel);
    int length();
    int freespace();
    void pushN(short *buf_in, int N);
    int pullN(int N,short *buf0);
    void triggerempty(void);
    void addloop();
    void removeloop();

private:
    int connected_loops;//nbr of loops connected to this ringbuffer

signals:
    void signal_trigger(void);

private slots:
    void data_available(short *buf,int nread);



};

#endif // RINGBUF_C_H





