#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_capture.h"
#include <stdbool.h>

#include "capture_port_c.h"

#include <QFile>
#include <QTextStream>

#include <qdebug.h>



#include "poll.h"


snd_pcm_t *capture_handle;
int capture_rate, capture_channels;
snd_pcm_uframes_t capture_frames,capture_period_size,capture_hw_buffersize;
capture_port_c** main_buf_capture;
short **capture_buf;
bool recording;
ConsumerCapture *consumerCapture;

void alsa_start_capture(QString device, int channels, int rate)
{

    if (!alsa_open_device_capture(device)) return;


    alsa_init_capture(channels,rate);
    alsa_set_hw_parameters_capture();
    alsa_set_sw_parameters_capture();

    alsa_begin_capture(main_buf_capture);

}

bool alsa_open_device_capture(QString device)
{
    int err;
    device = "plug"+device;//allows non interleaving
    if ((err = snd_pcm_open (&capture_handle,device.toStdString().c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {

        if(err == -EBUSY)
        {

            qDebug()<<"Device " + device + " busy";
            return false;
        }
        else
        {


            fprintf (stderr, "cannot open audio device %s for capture (%s)\n",
                     device.toStdString().c_str(),
                     snd_strerror (err));

            exit (1);
            return false;
        }
    }

    return true;

}

void alsa_init_capture(int channels,int rate)
{

    capture_channels = channels;
    capture_rate = rate;
    recording = true;

    //todo fix this
    capture_frames = CAPTURE_CHANNEL_WIDTH;
    capture_period_size = capture_frames*capture_channels;



    capture_buf = (short**)malloc(capture_channels*sizeof(short*));
    main_buf_capture = (capture_port_c**)malloc(channels*sizeof(capture_port_c));

    for(int i =0;i<channels;i++)
    {
        main_buf_capture[i] = new capture_port_c(RINGBUFSIZE_CAPTURE,capture_frames*capture_channels,rate,i);
        capture_buf[i] = main_buf_capture[i]->buf;
    }



}

void alsa_set_hw_parameters_capture(void)
{

    int err;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate = capture_rate;


    capture_hw_buffersize = capture_channels*CAPTURE_HW_BUFFER_SIZE;

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }



    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SOUND_FORMAT)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, capture_channels)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }




    snd_pcm_hw_params_set_buffer_size_near 	( 	capture_handle,hw_params,&capture_hw_buffersize);




    snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, &capture_period_size,0);





    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }




    snd_pcm_hw_params_free (hw_params);

}

void alsa_set_sw_parameters_capture(void)
{
    int err;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);

    err = snd_pcm_sw_params_current(capture_handle, swparams);

    if (err < 0) {
        fprintf(stderr, "Broken configuration for this PCM: no configurations available\n");
        exit(1);
    }

 /*   //threshold setting the ammount of data in the device buffer required for Alsa to stream the sound from the device
    err = snd_pcm_sw_params_set_start_threshold(capture_handle, swparams,  CAPTURE_SW_THRESHOLD );
    if (err < 0) {
        printf("Unable to set start threshold: %s\n", snd_strerror(err));
        exit(1);
    }
*/
    //when the device buffer data is smaller than this limit, an interrupt is issued
    err = snd_pcm_sw_params_set_avail_min(capture_handle, swparams, CAPTURE_AVAIL_MIN);
    if (err < 0) {
        printf("Unable to set avail min: %s\n", snd_strerror(err));
        exit(1);
    }

    if (snd_pcm_sw_params(capture_handle, swparams) < 0) {
        fprintf(stderr, "unable to install sw params:\n");
        exit(1);
    }

}

void alsa_begin_capture(capture_port_c **port)
{

    int err;


    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    snd_pcm_start(capture_handle); //Start the device

    consumerCapture = new ConsumerCapture();
    consumerCapture->port = port;
    consumerCapture->start();


  /*  snd_async_handler_t *pcm_callback;
    snd_async_add_pcm_handler(&pcm_callback,capture_handle,alsa_async_callback_capture,port);


    for(int i = 0; i < 3 ; i ++)
    {

        alsa_read_capture(port);


    }*/

}

void alsa_async_callback_capture(snd_async_handler_t *ahandler)
{


    snd_pcm_uframes_t avail;

    snd_pcm_t *capture_handle = snd_async_handler_get_pcm(ahandler);
    capture_port_c **port = (capture_port_c**)snd_async_handler_get_callback_private(ahandler);

    avail = snd_pcm_avail_update(capture_handle);

    while(avail >= capture_frames)
    {
        alsa_read_capture(port);
        avail = snd_pcm_avail_update(capture_handle);
    }



}

void alsa_read_capture(capture_port_c **port)
{


    int err;

    if ((err = snd_pcm_readn (capture_handle, (void**)capture_buf,capture_frames))!=(snd_pcm_sframes_t)capture_frames) {
        if(err == -EPIPE)
        {

            qDebug()<<"overrun capture";

            if ((err = snd_pcm_prepare (capture_handle)) < 0) {
                qDebug()<<"cannot prepare audio interface for use " << snd_strerror (err);
                exit (1);
            }
        }
        else
        {
            qDebug()<<"play on audio interface failed ";
            qDebug()<<err;
            exit(0);
        }
    }
    else
    {



        for(int i = 0;i<capture_channels;i++)
        {
            port[i]->pushN((unsigned long)capture_frames);

        }
    }


}

capture_port_c* alsa_capture_port_by_num(int channel)   {return main_buf_capture[channel];}

void alsa_start_capture_record(int channel,QString filename)
{
    capture_port_c *pcap = alsa_capture_port_by_num(channel);
    pcap->startrecord(filename);
}

void alsa_stop_capture_record(int channel)
{

    capture_port_c *pcap = alsa_capture_port_by_num(channel);
    pcap->stoprecord();
}

void alsa_cleanup_capture()
{


    recording = false;

    QThread::msleep(100); //give some time to the consumer thread to stop quietly
    consumerCapture->quit();
    snd_pcm_close(capture_handle);
    free(capture_buf);
    free(main_buf_capture);
    //qDebug()<<"cleaning up";
}

void alsa_monitor_capture(int channel, unsigned long *data)
{

    *data = alsa_capture_port_by_num(channel)->freespace();

}
















int wait_for_poll_IN(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count)
{
    unsigned short revents;
    while (recording) {

        poll(ufds, count, -1);

        snd_pcm_poll_descriptors_revents(handle, ufds, count, &revents);

        if (revents & POLLERR)
            return -EIO;
        if (revents & POLLIN)
            return 0;
    }
}

void read_and_poll_loop(capture_port_c **port)
{
    struct pollfd *ufds;


    int err, count;
    count = snd_pcm_poll_descriptors_count (capture_handle);

    if (count <= 0) {

        qDebug()<<"Invalid poll descriptors count\n";
        return ;
    }

    ufds = (struct pollfd*)malloc(sizeof(struct pollfd) * count);
    if (ufds == NULL) {
        qDebug()<<"No enough memory\n";
        return ;
    }
    if ((err = snd_pcm_poll_descriptors(capture_handle, ufds, count)) < 0) {
        qDebug()<<"Unable to obtain poll descriptors for playback: "<<snd_strerror(err);
        return ;
    }




    while (recording)
    {
        err = wait_for_poll_IN(capture_handle, ufds, count);
        if (err < 0) {qDebug()<<"capture polling error: ";return;}
        else
        {

            break;
        }

    }


    snd_pcm_uframes_t avail;
    avail = snd_pcm_avail_update(capture_handle);


    while(avail >= capture_frames)
    {
        alsa_read_capture(port);
        avail = snd_pcm_avail_update(capture_handle);
    }

}

void ConsumerCapture::run()
{

    while(recording)
    {


        read_and_poll_loop(port);
        QThread::usleep(1500);

    }

}








