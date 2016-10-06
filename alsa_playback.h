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

#endif // ALSA_PLAYBACK_H
