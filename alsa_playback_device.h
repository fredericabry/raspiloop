#ifndef ALSA_DEVICE_H
#define ALSA_DEVICE_H


#include <alsa/asoundlib.h>
#include "playback_port_c.h"
#include <qthread.h>
#include "interface.h"

class alsa_playback_device;



class ConsumerDevicePlayback:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

 public:
    playback_port_c **port;
    alsa_playback_device *controler;
    void write_and_poll_loop(playback_port_c **port);
    int wait_for_poll(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count);
    bool playing;
};

class alsa_playback_device:public QObject
{
    Q_OBJECT

public:
    alsa_playback_device(QString device, int channels, int rate, interface_c *interface, bool *success);

    short **playback_buf;
    QString deviceName;
    interface_c *pInterface;
    ConsumerDevicePlayback *consumer;
    snd_pcm_t *playback_handle;
    snd_pcm_uframes_t playback_frames,playback_period_size,playback_hw_buffersize;
    int playback_rate;
    int playback_channels;
    playback_port_c** main_buf_playback;

   // void alsa_start_playback(QString device, int channels, int rate,interface_c *interface);
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

private:
    QString deviceDesc;


};

#endif // ALSA_DEVICE_H
