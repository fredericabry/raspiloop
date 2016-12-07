#ifndef ALSA_UTIL_H
#define ALSA_UTIL_H


#include <alsa/asoundlib.h>
#include "qstringlist.h"



#include <qmutex.h>


struct hw_info
{
    QString name;
    QString longname;
    short max_port;
    short min_port;
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




void getCardList(snd_pcm_stream_t,QStringList*, QStringList*);
int getCardListLength(snd_pcm_stream_t);
QStringList pcm_list(snd_pcm_stream_t);
struct hw_info get_device_info(QString, snd_pcm_stream_t stream);

QString getCardDescription(QString, snd_pcm_stream_t stream);



#endif // ALSA_UTIL_H
