#ifndef ALSA_UTIL_H
#define ALSA_UTIL_H


#include <alsa/asoundlib.h>
#include "qstringlist.h"



#include <qmutex.h>
static QMutex filelock;

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




void getCardList(snd_pcm_stream_t,QStringList*, QStringList*);
int getCardListLength(snd_pcm_stream_t);
QStringList pcm_list(snd_pcm_stream_t);
struct hw_info get_device_info(QString);





#endif // ALSA_UTIL_H
