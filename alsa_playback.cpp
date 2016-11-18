#include "alsa_playback.h"
#include <qdebug.h>
#include "qthread.h"
#include "parameters.h"


short **playback_buf;


bool playing;


ConsumerPlayback *consumer;


snd_pcm_t *playback_handle;
snd_pcm_uframes_t playback_frames,playback_period_size,playback_hw_buffersize;

int playback_rate;
int playback_channels;


playback_port_c** main_buf_playback;


void alsa_start_playback(QString device, int channels, int rate, interface_c *interface)
{



    if (!alsa_open_device_playback(device)) return;


    alsa_init_playback(channels,rate,interface);


    alsa_set_hw_parameters_playback();
    alsa_set_sw_parameters_playback();
    alsa_begin_playback(main_buf_playback);

}

bool alsa_open_device_playback(QString device)
{
    int err;
    device = "plug"+device;//allow non interleaving
    if ((err = snd_pcm_open (&playback_handle,device.toStdString().c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {

        if(err == -EBUSY)
        {

            qDebug()<<"Device " + device + " busy";
            return false;
        }
        else
        {


            fprintf (stderr, "cannot open audio device %s (%s)\n",
                     device.toStdString().c_str(),
                     snd_strerror (err));

            exit (1);
            return false;
        }
    }

    return true;

}

void alsa_init_playback(int channels,int rate,interface_c *interface)
{

    playback_channels = channels;
    playback_rate = rate;

    //todo fix this
    playback_frames = PLAYBACK_CHANNEL_WIDTH;
    playback_period_size = playback_frames*playback_channels;

    playing = true;




    main_buf_playback = (playback_port_c**)malloc(channels*sizeof(playback_port_c));
    playback_buf = (short**)malloc(playback_channels*sizeof(short*));


    for(int i =0;i<channels;i++)
    {
        main_buf_playback[i] = new playback_port_c(RINGBUFSIZE_PLAYBACK,PLAYBACK_BUFSIZE,i,interface);
        playback_buf[i] = main_buf_playback[i]->buf;
    }





}

void alsa_set_hw_parameters_playback(void)
{

    int err;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate = playback_rate;


    playback_hw_buffersize = playback_channels*PLAYBACK_HW_BUFFER_SIZE;

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }



    if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SOUND_FORMAT)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, playback_channels)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }




    snd_pcm_hw_params_set_buffer_size_near 	( 	playback_handle,hw_params,&playback_hw_buffersize);




    snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &playback_period_size,0);





    if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }




    snd_pcm_hw_params_free (hw_params);

}


void alsa_set_sw_parameters_playback(void)
{
    int err;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);

    err = snd_pcm_sw_params_current(playback_handle, swparams);

    if (err < 0) {
        fprintf(stderr, "Broken configuration for this PCM: no configurations available\n");
        exit(1);
    }

    //threshold setting the ammount of data in the device buffer required for Alsa to allow interupts
    err = snd_pcm_sw_params_set_avail_min(playback_handle, swparams, playback_channels*PLAYBACK_AVAIL_MIN);
    if (err < 0) {
        printf("Unable to set start threshold: %s\n", snd_strerror(err));
        exit(1);
    }




    if (snd_pcm_sw_params(playback_handle, swparams) < 0) {
        fprintf(stderr, "unable to install sw params:\n");
        exit(1);
    }

}

void alsa_begin_playback(playback_port_c **port)
{

    int err;


    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


   snd_pcm_start(playback_handle); //Start the device

    //   snd_async_handler_t *pcm_callback;
    //   snd_async_add_pcm_handler(&pcm_callback,playback_handle,alsa_async_callback_playback,port);

    consumer = new ConsumerPlayback();
    consumer->port = port;
    consumer->start();





}

void alsa_write_playback(playback_port_c **port)
{
    int err;



    for(int i = 0;i<playback_channels;i++)
    {
        port[i]->pullN(playback_frames);

    }



    if ((err = snd_pcm_writen (playback_handle, (void**)playback_buf,playback_frames))!=(snd_pcm_sframes_t)playback_frames) {
        if(err == -EPIPE)
        {

            qDebug()<<"underrun playback " ;

            if ((err = snd_pcm_prepare (playback_handle)) < 0) {
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

}


void alsa_conf(void)
{
    snd_pcm_uframes_t  	buffer_size;
    snd_pcm_uframes_t period_size;
    snd_pcm_get_params 	( 	playback_handle,       &buffer_size, 	&period_size );

}

playback_port_c* alsa_playback_port_by_num(int channel)
{


    return main_buf_playback[channel];
}

void alsa_cleanup_playback()
{
    playing = false;




    QThread::msleep(100); //give some time to the consumer thread to stop quietly
    consumer->quit();
    snd_pcm_close(playback_handle);
    free(playback_buf);
    free(main_buf_playback);

    //qDebug()<<"cleaning up";
}

int wait_for_poll(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count)
{
    unsigned short revents;
    while (1) {
        poll(ufds, count, -1);
        snd_pcm_poll_descriptors_revents(handle, ufds, count, &revents);
        if (revents & POLLERR)
            return -EIO;
        if (revents & POLLOUT)
            return 0;
    }
}

void write_and_poll_loop(playback_port_c **port)
{
    struct pollfd *ufds;

    int err, count;
    count = snd_pcm_poll_descriptors_count (playback_handle);
    if (count <= 0) {
        qDebug()<<"Invalid poll descriptors count\n";
        return ;
    }

    ufds = (struct pollfd*)malloc(sizeof(struct pollfd) * count);
    if (ufds == NULL) {
        qDebug()<<"No enough memory\n";
        return ;
    }
    if ((err = snd_pcm_poll_descriptors(playback_handle, ufds, count)) < 0) {
        qDebug()<<"Unable to obtain poll descriptors for playback: "<<snd_strerror(err);
        return ;
    }





    while (playing) {
        err = wait_for_poll(playback_handle, ufds, count);
        if (err < 0) {qDebug()<<"playback polling error";return;}
        else
        {
            break;
        }

    }


    snd_pcm_uframes_t avail;
    avail = snd_pcm_avail_update(playback_handle);

   // qDebug()<<avail;
    while(avail >= playback_frames)
    {
        alsa_write_playback(port);
        avail = snd_pcm_avail_update(playback_handle);
    }

}

void ConsumerPlayback::run()
{

    while(playing)
    {
        write_and_poll_loop(port);
        QThread::usleep(PLAYBACK_READBUF_SLEEP);
    }
}








