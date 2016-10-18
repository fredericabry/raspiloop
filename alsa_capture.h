#ifndef ALSA_CAPTURE
#define ALSA_CAPTURE



#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "mainwindow.h"
#include "capture_port_c.h"




#define CAPTURE_CHANNEL_WIDTH 256 //number of elements in a a frame for ONE channel
#define CAPTURE_HW_BUFFER_SIZE 4*CAPTURE_CHANNEL_WIDTH //alsa capture buffer size for ONE channel
#define RINGBUFSIZE_CAPTURE 10000 //size of the capture ringbuffer
#define NFILE_CAPTURE CAPTURE_CHANNEL_WIDTH*10//file writting buffer size

#define TRIGGER_CAPTURE RINGBUFSIZE_CAPTURE/4   //device starts to stream to file when ringbuf length > this value



#define CAPTURE_INTERRUPT_THRESHOLD        0.3*CAPTURE_HW_BUFFER_SIZE //when the device buffer data is bigger than this limit, an interrupt is issued


#define CAPTURE_SW_THRESHOLD               1  //threshold setting the ammount of data in the device buffer required for Alsa to stream the sound to the device


void alsa_start_capture(QString device, int channels, int rate);
bool alsa_open_device_capture(QString device);
void alsa_init_capture(int channels,int rate);






void alsa_set_hw_parameters_capture(void);
void alsa_set_sw_parameters_capture(void);
void alsa_async_callback_capture(snd_async_handler_t *ahandler);
void alsa_begin_capture(capture_port_c **port);
void alsa_read_capture(capture_port_c **port);
void alsa_start_capture_record(int channel,QString filename);
void alsa_stop_capture_record(int channel);
void alsa_cleanup_capture(void);
void alsa_monitor_capture(int channel, unsigned long *data);
capture_port_c* alsa_capture_port_by_num(int channel);

#endif // ALSA_CAPTURE

