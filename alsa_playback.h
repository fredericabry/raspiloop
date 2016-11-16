#ifndef ALSA_PLAYBACK_H
#define ALSA_PLAYBACK_H

#include <alsa/asoundlib.h>
#include "playback_port_c.h"
#include <qthread.h>
#include "interface.h"


class ConsumerPlayback:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

 public:
    playback_port_c **port;
};


void alsa_start_playback(QString device, int channels, int rate,interface_c *interface);
bool alsa_open_device_playback(QString device);
void alsa_init_playback(int channels, int rate, interface_c *interface);
void alsa_set_hw_parameters_playback(void);
void alsa_set_sw_parameters_playback(void);
void alsa_async_callback_playback(snd_async_handler_t *ahandler);
void alsa_begin_playback(playback_port_c**);
void alsa_write_playback(playback_port_c**);
void alsa_conf(void);
void alsa_cleanup_playback(void);
playback_port_c* alsa_playback_port_by_num(int channel);
void alsa_monitor_playback(int channel,unsigned long *data);

void write_and_poll_loop(playback_port_c **port);

#endif // ALSA_PLAYBACK_H
