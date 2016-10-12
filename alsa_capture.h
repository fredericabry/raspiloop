#ifndef ALSA_CAPTURE
#define ALSA_CAPTURE



#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "mainwindow.h"
#include "capture_port_c.h"
#include "loop_c.h"





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



#endif // ALSA_CAPTURE

