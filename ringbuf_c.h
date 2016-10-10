#ifndef RINGBUF_C_H
#define RINGBUF_C_H

#include <sndfile.h>
#include <alsa/asoundlib.h>

class ringbuf_c
{
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


    SNDFILE *soundfile;


    int push(short data);
    int pull(short *data);
    void fill(int channel);
    void ringbuf_fill(int channel);
    int length();
    int freespace();
    void pushN(short *buf_in, int N);
    int pullN(int N,short *buf0);
    void triggerempty(void);
    void soundupd(void);



};

#endif // RINGBUF_C_H





