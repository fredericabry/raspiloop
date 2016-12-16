#include "alsa_capture_device.h"
#include <qdebug.h>
#include "alsa_util.h"


alsa_capture_device::alsa_capture_device(QString device,int channels, int rate,interface_c *interface,bool *success):deviceName(device)
{


    deviceDesc = getCardDescription(device,SND_PCM_STREAM_CAPTURE);


    if (!alsa_open_device_capture(device)) return;



    alsa_init_capture(channels,rate,interface);
    alsa_set_hw_parameters_capture();
    alsa_set_sw_parameters_capture();

    alsa_begin_capture(main_buf_capture);


    *success = true;

}

bool alsa_capture_device::alsa_open_device_capture(QString device)
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


          qDebug()<<"cannot open audio device"<<device.toStdString().c_str()<<"for capture"<<snd_strerror (err);

          //  exit (1);
            return false;
        }
    }

    return true;

}

void alsa_capture_device::alsa_init_capture(int channels,int rate,interface_c *interface)
{

    capture_channels = channels;
    capture_rate = rate;


    //todo fix this
    capture_frames = CAPTURE_CHANNEL_WIDTH;
    capture_period_size = capture_frames*capture_channels;



    capture_buf = (short**)malloc(capture_channels*sizeof(short*));
    main_buf_capture = (capture_port_c**)malloc(channels*sizeof(capture_port_c));

    for(int i =0;i<channels;i++)
    {
        interface->capturePortsCount++;
        main_buf_capture[i] = new capture_port_c(RINGBUFSIZE_CAPTURE,capture_frames, interface->capturePortsCount,deviceDesc,interface);
        capture_buf[i] = main_buf_capture[i]->bufin;
    }



}

void alsa_capture_device::alsa_set_hw_parameters_capture(void)
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

void alsa_capture_device::alsa_set_sw_parameters_capture(void)
{
    int err;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);

    err = snd_pcm_sw_params_current(capture_handle, swparams);

    if (err < 0) {
        fprintf(stderr, "Broken configuration for this PCM: no configurations available\n");
        exit(1);
    }


    //when the device buffer data is bigger than this limit, an interrupt is issued
    err = snd_pcm_sw_params_set_avail_min(capture_handle, swparams, capture_channels*CAPTURE_AVAIL_MIN);
    if (err < 0) {
        printf("Unable to set avail min: %s\n", snd_strerror(err));
        exit(1);
    }





    if (snd_pcm_sw_params(capture_handle, swparams) < 0) {
        fprintf(stderr, "unable to install sw params:\n");
        exit(1);
    }

}

void alsa_capture_device::alsa_begin_capture(capture_port_c **port)
{

    int err;


    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    snd_pcm_start(capture_handle); //Start the device

    consumer = new ConsumerDeviceCapture();
    consumer->recording = true;
    consumer->controler = this;
    consumer->consumerLock = false;
    consumer->port = port;
    consumer->start();

}

void alsa_capture_device::alsa_read_capture(capture_port_c **port)
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

            snd_pcm_start(capture_handle);
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

capture_port_c* alsa_capture_device::alsa_capture_port_by_num(int channel)   {return main_buf_capture[channel];}

void alsa_capture_device::alsa_cleanup_capture()
{
    consumer->recording = false;



}

int ConsumerDeviceCapture::wait_for_poll_IN(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count)
{

    unsigned short revents;
    while (recording) {


        poll(ufds, count, -1);

        snd_pcm_poll_descriptors_revents(handle, ufds, count, &revents);

        if (revents & POLLERR)
            return -EIO;
        if (revents & POLLIN)
        {

            return 0;
        }
    }


    qDebug()<<"bug wait for poll IN";
    return 0;
}

void ConsumerDeviceCapture::read_and_poll_loop(capture_port_c **port)
{




    if(consumerLock) {qDebug()<<"overlap read capture";return;}

    consumerLock = true;

    struct pollfd *ufds;


    int err, count;
    count = snd_pcm_poll_descriptors_count (controler->capture_handle);

    if (count <= 0) {

        qDebug()<<"Invalid poll descriptors count\n";
        return ;
    }

    ufds = (struct pollfd*)malloc(sizeof(struct pollfd) * count);
    if (ufds == NULL) {
        qDebug()<<"No enough memory\n";
        return ;
    }
    if ((err = snd_pcm_poll_descriptors(controler->capture_handle, ufds, count)) < 0) {
        qDebug()<<"Unable to obtain poll descriptors for playback: "<<snd_strerror(err);
        return ;
    }



    while (recording)
    {

        err = wait_for_poll_IN(controler->capture_handle, ufds, count);
        if (err < 0) {qDebug()<<"capture polling error: ";return;}
        else
        {

            break;
        }

    }



    snd_pcm_uframes_t avail;
    avail = snd_pcm_avail_update(controler->capture_handle);

    //    qDebug()<<avail;

    while(avail >= controler->capture_frames)
    {
        controler->alsa_read_capture(port);
        avail = snd_pcm_avail_update(controler->capture_handle);

    }

    consumerLock = false;

}

void ConsumerDeviceCapture::run()
{




    while(recording)
    {
        read_and_poll_loop(port);
        QThread::usleep(CAPTURE_READBUF_SLEEP);

    }

    snd_pcm_close(controler->capture_handle);
    free(controler->capture_buf);
    free(controler->main_buf_capture);
    controler->deleteLater();



}








