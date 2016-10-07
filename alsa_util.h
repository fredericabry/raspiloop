#ifndef ALSA_UTIL_H
#define ALSA_UTIL_H


#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "mainwindow.h"

#define SOUND_FORMAT SND_PCM_FORMAT_S16_LE


struct hw_info
{
    QString name;
    QString longname;
    short max_capture;
    short min_capture;
    short max_playback;
    short min_playback;
    int max_rate;
    int min_rate;
    int min_period_time;
    int max_period_time;
    int min_period_size;
    int max_period_size;
    int min_period_per_buffer;
    int max_period_per_buffer;
    int min_buffer_time;
    int max_buffer_time;
    int min_buffer_size;
    int max_buffer_size;

};




typedef struct
{
    short * const buf;
    int head;
    int tail;
    const int maxlength;
}ringbuf_t;



int ringbuf_push(ringbuf_t *ringbuf,short data);
int ringbuf_pull(ringbuf_t *ringbuf,short *data);
void ringbuf_copy(ringbuf_t *ringbuf_in, short *buf_out, int nelements);
void ringbuf_fill(ringbuf_t *ringbuf_in);
int ringbuf_length(ringbuf_t *ringbuf);

#define RINGBUF_DEF(bufname,length) short bufname##_data[length+1]; ringbuf_t bufname = {bufname##_data,0,0,length}







void getCardList(snd_pcm_stream_t,QStringList*, QStringList*);
int getCardListLength(snd_pcm_stream_t);
QStringList pcm_list(snd_pcm_stream_t);
struct hw_info get_device_info(QString);




void async_callback_record(snd_async_handler_t *ahandler);
void open_file_record(QString filename);
void set_hw_parameters_record(QString device);
void set_sw_parameters_record(void);
void start_record(void);
void alsa_record(QString device, int channels, int rate, long length, QString filename, MainWindow *pt);
void init_record(int channels,int rate, long length);
void stop_record(void);


void async_callback_play(snd_async_handler_t *ahandler);
void open_file_play(QString filename);
void set_hw_parameters_play(void);
void set_sw_parameters_play(void);
void start_play(void);
void alsa_play(QString device, int channels, int rate, long length, QString filename, MainWindow *pt);
void init_play(int channels,int rate, long length);
void stop_play(void);
bool open_device_play(QString device);











#endif // ALSA_UTIL_H
