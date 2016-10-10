#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>

#include "ringbuf_c.h"

#include <QFile>
#include <QTextStream>

#include <qdebug.h>




#define CHANNEL_WIDTH 256 //number of elements in a a frame for ONE channel
#define FRAME_PER_BUFFER 5 //size of the buffer in frame






MainWindow *parent;
snd_pcm_t *playback_handle;
snd_pcm_uframes_t playback_frames,playback_bufsize,playback_period_size;
snd_pcm_uframes_t hw_buffersize;

short *empty_buf;

int playback_rate;
int playback_channels;
//short *playback_buf;
short *buf1,*buf2;


SNDFILE *sf_play;

ringbuf_t** main_buf_playback;
RINGBUF_DEF(left_buf_playback,500000);
RINGBUF_DEF(right_buf_playback,500000);





void alsa_start_playback(QString device, int channels, int rate, MainWindow *pt)
{

    if (!alsa_open_device_playback(device)) return;

    parent= pt;
    alsa_init_playback(channels,rate);
    //open_file_play(filename);
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

void alsa_init_playback(int channels,int rate)
{

    playback_channels = channels;
    playback_rate = rate;

    //todo fix this
    playback_frames = CHANNEL_WIDTH;
    playback_period_size = playback_frames*playback_channels;
    playback_bufsize = FRAME_PER_BUFFER*playback_frames;


    empty_buf = (short*)malloc(playback_period_size*sizeof(short));

    for(int i = 0; i<(int)playback_period_size;i++) empty_buf[i] = 0;


    //playback_buf = (short*)malloc(sizeof(short)*playback_frames*2);
    buf1 = (short*)malloc(sizeof(short)*playback_frames*2);
    buf2 = (short*)malloc(sizeof(short)*playback_frames*2);



    main_buf_playback = (ringbuf_t**)malloc(channels*sizeof(ringbuf_t*));
    main_buf_playback[0] = &left_buf_playback;
    main_buf_playback[1] = &right_buf_playback;




}

void alsa_set_hw_parameters_playback(void)
{

    int err;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate = playback_rate;


    hw_buffersize = 2*playback_channels*CHANNEL_WIDTH;

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




    snd_pcm_hw_params_set_buffer_size_near 	( 	playback_handle,hw_params,&hw_buffersize);




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

void alsa_write_playback(ringbuf_t **ringbuf)
{
    int err;
    short *playback_buf[2];

    ringbuf_pullN(ringbuf[0],buf1,playback_frames,empty_buf);
    ringbuf_pullN(ringbuf[1],buf2,playback_frames,empty_buf);

    playback_buf[0] = buf1;
    playback_buf[1] = buf2;

        if ((err = snd_pcm_writen (playback_handle, (void**)playback_buf,playback_frames))!=(snd_pcm_sframes_t)playback_frames) {

            qDebug()<<"play on audio interface failed ";
            exit(0);
        }

}

void alsa_async_callback_playback(snd_async_handler_t *ahandler)
{


    snd_pcm_uframes_t avail;

    snd_pcm_t *playback_handle = snd_async_handler_get_pcm(ahandler);
    ringbuf_t **ringbuf = (ringbuf_t**)snd_async_handler_get_callback_private(ahandler);

    avail = snd_pcm_avail_update(playback_handle);

    while(avail >= playback_frames)
    {
        alsa_write_playback(ringbuf);
        avail = snd_pcm_avail_update(playback_handle);
    }

    //qDebug()<<"a";

}

void alsa_begin_playback(ringbuf_t **ringbuf)
{

    int err;


    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }




    snd_async_handler_t *pcm_callback;
    snd_async_add_pcm_handler(&pcm_callback,playback_handle,alsa_async_callback_playback,ringbuf);


    for(int i = 0; i < 3 ; i ++)
    {

        alsa_write_playback(ringbuf);


    }

}

void alsa_conf(void)
{
    snd_pcm_uframes_t  	buffer_size;
    snd_pcm_uframes_t period_size;
    snd_pcm_get_params 	( 	playback_handle,       &buffer_size, 	&period_size );

}

void alsa_load_file(int channel)
{
    SF_INFO sf_info;
    int nread;
    short *buf;
    sf_info.format = 0;
    if ((sf_play = sf_open ("ding2.wav", SFM_READ, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile for output %s\n", errstr);

        exit (1);
    }

    int short_mask = SF_FORMAT_PCM_16;

    if(sf_info.format != (SF_FORMAT_WAV|short_mask))
    {
        qDebug()<<"format de fichier incorrect\n";
        return;
    }
    /*
    if(sf_info.samplerate != rate_play)
    {
        parent_play->Afficher("soundfile rate incorrect\n");
        return;
    }
    */
    if(sf_info.channels != 1)
    {
        qDebug()<<"chan nbr incorrect\n";
        return;
    }

    buf = (short*)malloc(30000*sizeof(short));





    if((nread = sf_readf_short(sf_play,buf,15000))>0)
    {

        ringbuf_pushN(main_buf_playback[channel],buf,nread);

    }


}
