#ifndef ALSA_UTIL_H
#define ALSA_UTIL_H


#include <alsa/asoundlib.h>
#include <sndfile.h>




void getCardList(snd_pcm_stream_t,QStringList*, QStringList*);
int getCardListLength(snd_pcm_stream_t);
QStringList pcm_list(snd_pcm_stream_t);
QString info(QString, snd_pcm_stream_t );
void async_callback(snd_async_handler_t *ahandler);
void open_file(QString filename);
void set_hw_parameters(QString device);
void set_sw_parameters(void);
void start_record(void);
void alsa_record(QString device, int channels, int rate, long length, QString filename, MainWindow *pt);
void init_record(int channels);










#endif // ALSA_UTIL_H
