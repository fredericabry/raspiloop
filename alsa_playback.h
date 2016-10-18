#ifndef ALSA_PLAYBACK_H
#define ALSA_PLAYBACK_H

#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "mainwindow.h"
#include "playback_port_c.h"
#include "loop_c.h"



#define PLAYBACK_CHANNEL_WIDTH 256 //number of elements in a a frame for ONE channel




#define NFILE_PLAYBACK PLAYBACK_CHANNEL_WIDTH*10 //file reading buffer size


#define RINGBUFSIZE_PLAYBACK 5000 //size of the playback ringbuffer
#define THRESHOLD RINGBUFSIZE_PLAYBACK/2 //interrupt when length(ringbuffer) < this value



#define PLAYBACK_HW_BUFFER_SIZE             4*PLAYBACK_CHANNEL_WIDTH  //alsa playback buffer size for ONE channel
#define PLAYBACK_SW_THRESHOLD               PLAYBACK_HW_BUFFER_SIZE/4  //threshold setting the ammount of data in the device buffer required for Alsa to stream the sound to the device
#define PLAYBACK_INTERRUPT_THRESHOLD        1*PLAYBACK_HW_BUFFER_SIZE/4 //when the device buffer data is smaller than this limit, an interrupt is issued





void alsa_start_playback(QString device, int channels, int rate);
bool alsa_open_device_playback(QString device);
void alsa_init_playback(int channels, int rate);
void alsa_set_hw_parameters_playback(void);
void alsa_set_sw_parameters_playback(void);
void alsa_async_callback_playback(snd_async_handler_t *ahandler);
void alsa_begin_playback(playback_port_c**);
void alsa_write_playback(playback_port_c**);
void alsa_conf(void);
void alsa_cleanup_playback(void);
playback_port_c* alsa_playback_port_by_num(int channel);
void alsa_monitor_playback(int channel,unsigned long *data);

#endif // ALSA_PLAYBACK_H
