#ifndef ALSA_CAPTURE
#define ALSA_CAPTURE



#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "mainwindow.h"
#include "capture_port_c.h"
#include "qmutex.h"
#include "qthread.h"
#include <QMainWindow>


#define CAPTURE_CHANNEL_WIDTH 256 //number of elements in a a frame for ONE channel
#define CAPTURE_HW_BUFFER_SIZE 3*CAPTURE_CHANNEL_WIDTH //alsa capture buffer size for ONE channel
#define RINGBUFSIZE_CAPTURE 44100*3 //size of the capture ringbuffer : 3 seconds
#define NFILE_CAPTURE CAPTURE_CHANNEL_WIDTH*5//file writting buffer size

#define CAPTURE_READBUF_SLEEP (unsigned long)5000 //usec

#define CAPTURE_WRITEFILE_SLEEP (unsigned long)5000 //us

#define CAPTURE_AVAIL_MIN       5*CAPTURE_HW_BUFFER_SIZE/10 //when the device buffer data is bigger than this limit, an interrupt is issued

#define CAPTURE_LOOP_BUFSIZE 44100 //number of elements of each capture loop ring buffer



class ConsumerCapture:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

 public:
    capture_port_c **port;


};






void alsa_start_capture(QString device, int channels, int rate);
bool alsa_open_device_capture(QString device);
void alsa_init_capture(int channels,int rate);






void alsa_set_hw_parameters_capture(void);
void alsa_set_sw_parameters_capture(void);
void alsa_async_callback_capture(snd_async_handler_t *ahandler);
void alsa_begin_capture(capture_port_c **port);
void alsa_read_capture(capture_port_c **port);

void alsa_cleanup_capture(void);
void alsa_monitor_capture(int channel, unsigned long *data);
capture_port_c* alsa_capture_port_by_num(int channel);

#endif // ALSA_CAPTURE

