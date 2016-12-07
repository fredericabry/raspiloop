#ifndef ALSA_CAPTURE_DEVICE_H
#define ALSA_CAPTURE_DEVICE_H

#include <alsa/asoundlib.h>
#include "capture_port_c.h"
#include "qthread.h"
#include "parameters.h"
#include "interface.h"

class alsa_capture_device;

class ConsumerDeviceCapture:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

public:
    capture_port_c **port;
    alsa_capture_device *controler;
    int wait_for_poll_IN(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count);
    void read_and_poll_loop(capture_port_c **port);
    bool recording;
    bool consumerLock;
};


class alsa_capture_device:public QObject
{
    Q_OBJECT

public:
    alsa_capture_device(QString device, int channels, int rate,interface_c *interface);
    bool alsa_open_device_capture(QString device);
    void alsa_init_capture(int channels,int rate,interface_c *interface);
    void alsa_set_hw_parameters_capture(void);
    void alsa_set_sw_parameters_capture(void);
    void alsa_async_callback_capture(snd_async_handler_t *ahandler);
    void alsa_begin_capture(capture_port_c **port);
    void alsa_read_capture(capture_port_c **port);
    void alsa_cleanup_capture(void);
    void alsa_monitor_capture(int channel, unsigned long *data);
    capture_port_c* alsa_capture_port_by_num(int channel);

    snd_pcm_t *capture_handle;
    int capture_rate, capture_channels;
    snd_pcm_uframes_t capture_frames,capture_period_size,capture_hw_buffersize;
    capture_port_c** main_buf_capture;
    short **capture_buf;
    ConsumerDeviceCapture *consumer;

private:
    QString deviceName;
    QString deviceDesc;
};

#endif // ALSA_CAPTURE_DEVICE_H














