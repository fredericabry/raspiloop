#ifndef ALSA_UTIL_H
#define ALSA_UTIL_H


#include <alsa/asoundlib.h>
#include <sndfile.h>




void getCardList(snd_pcm_stream_t,QStringList*, QStringList*);
int getCardListLength(snd_pcm_stream_t);
QStringList pcm_list(snd_pcm_stream_t);
QString info(QString, snd_pcm_stream_t );




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
void set_hw_parameters_play(QString device);
void set_sw_parameters_play(void);
void start_play(void);
void alsa_play(QString device, int channels, int rate, long length, QString filename, MainWindow *pt);
void init_play(int channels,int rate, long length);
void stop_play(void);












#endif // ALSA_UTIL_H
