#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>


#include <QFile>
#include <QTextStream>

#include <qdebug.h>




#define CHANNEL_WIDTH 128 //number of elements in a a frame for ONE channel
#define FRAME_PER_BUFFER 5 //size of the buffer in frame







MainWindow *parent;
snd_pcm_t *playback_handle;
snd_pcm_uframes_t playback_frames,playback_bufsize,playback_period_size;
short *playback_buf;
short *empty_buf;

int playback_rate;
int playback_channels;



ringbuf_t *ringbuf2;












void alsa_start_playback(QString device, int channels, int rate, MainWindow *pt,ringbuf_t *ringbuf)
{

    if (!alsa_open_device_playback(device)) return;

    parent= pt;
    alsa_init_playback(channels,rate);
    //open_file_play(filename);
    alsa_set_hw_parameters_playback();
    alsa_set_sw_parameters_playback();
    alsa_begin_playback(ringbuf);

}

bool alsa_open_device_playback(QString device)
{
    int err;

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

void alsa_init_playback(int channels,int rate)
{

    playback_channels = channels;
    playback_rate = rate;

    //todo fix this
    playback_frames = CHANNEL_WIDTH;
    playback_period_size = playback_frames*playback_channels;
    playback_bufsize = FRAME_PER_BUFFER*playback_frames;


    empty_buf = (short*)malloc(playback_period_size*sizeof(short));
    for(int i = 0; i<playback_period_size;i++) empty_buf[i] = 0;


}

void alsa_set_hw_parameters_playback(void)
{

    int err;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate = playback_rate;

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



    if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
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





     snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &playback_period_size,0);


    playback_buf =  (short*)malloc(playback_bufsize*sizeof(short));


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

    //threshold setting the ammount of data in the device buffer required for Alsa to stream the sound to the device
    err = snd_pcm_sw_params_set_start_threshold(playback_handle, swparams,  playback_frames);
    if (err < 0) {
        printf("Unable to set start threshold: %s\n", snd_strerror(err));
        exit(1);
    }

    //when the device buffer data is smaller than this limit, an interrupt is issued
    err = snd_pcm_sw_params_set_avail_min(playback_handle, swparams, playback_frames);
    if (err < 0) {
        printf("Unable to set avail min: %s\n", snd_strerror(err));
        exit(1);
    }

    if (snd_pcm_sw_params(playback_handle, swparams) < 0) {
        fprintf(stderr, "unable to install sw params:\n");
        exit(1);
    }

}

void alsa_write_playback(ringbuf_t *ringbuf)
{
    static int a;
    int nread,err;
    nread = ringbuf_pullN(ringbuf,playback_buf,playback_frames*playback_channels)/playback_channels;

    if(nread > 0)
    {
       // qDebug()<<nread;
        if ((err = snd_pcm_writei (playback_handle, playback_buf,nread))!=nread) {

            qDebug()<<"play on audio interface failed ";
            exit(0);
        }
    }
    else //fill in with 0 waiting for some sound to stream
    {
        if ((err = snd_pcm_writei (playback_handle, empty_buf,playback_frames))!=playback_frames) {

            qDebug()<<"play on audio interface failed ";
            exit(0);
        }

    }
}

void alsa_async_callback_playback(snd_async_handler_t *ahandler)
{

    int err;
    snd_pcm_uframes_t avail;

    snd_pcm_t *playback_handle = snd_async_handler_get_pcm(ahandler);


    avail = snd_pcm_avail_update(playback_handle);

    while(avail >= playback_frames)
    {


        alsa_write_playback(ringbuf2);

        avail = snd_pcm_avail_update(playback_handle);

    }

    //qDebug()<<"a";

}

void alsa_begin_playback(ringbuf_t *ringbuf)
{

    int err;


    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    ringbuf2 = ringbuf;


    snd_async_handler_t *pcm_callback;
    snd_async_add_pcm_handler(&pcm_callback,playback_handle,alsa_async_callback_playback,NULL);


    for(int i = 0; i < 3 ; i ++)
    {

        alsa_write_playback(ringbuf);


    }

}
