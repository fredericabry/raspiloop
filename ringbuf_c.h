#ifndef RINGBUF_C_H
#define RINGBUF_C_H


class ringbuf_c
{
public:
    ringbuf_c(const int maxlength,const int bufsize);
    ~ringbuf_c();
    short *ringbuf;//big buffer for circular storage
    short *buf;//small buffer for transfert


    int head;
    int tail;
    int const maxlength;
    int const bufsize;


    int push(short data);
    int pull(short *data);
    void fill(int channel);
    void ringbuf_fill(int channel);
    int length();
    int freespace();
    void pushN(short *buf_in, int N);
    int pullN(int N,short *buf0);

};

#endif // RINGBUF_C_H





