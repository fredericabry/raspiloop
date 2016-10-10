#ifndef ALSA_PLAYBACK_H
#define ALSA_PLAYBACK_H

#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "mainwindow.h"






void alsa_start_playback(QString device, int channels, int rate, MainWindow *pt);
void alsa_init_playback(int channels,int rate);
void alsa_set_hw_parameters_playback(void);
void alsa_set_sw_parameters_playback(void);
void alsa_async_callback_playback(snd_async_handler_t *ahandler);
void alsa_begin_playback(ringbuf_t **ringbuf);
void alsa_write_playback(ringbuf_t **ringbuf);
bool alsa_open_device_playback(QString device);
void alsa_conf(void);
void alsa_load_file(int channel);


#endif // ALSA_PLAYBACK_H