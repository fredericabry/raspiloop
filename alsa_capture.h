#ifndef ALSA_CAPTURE
#define ALSA_CAPTURE



#include <alsa/asoundlib.h>
#include "capture_port_c.h"
#include "qthread.h"
#include "parameters.h"
#include "interface.h"


class ConsumerCapture:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

 public:
    capture_port_c **port;


};


void alsa_start_capture(QString device, int channels, int rate,interface_c *interface);
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

#endif // ALSA_CAPTURE

