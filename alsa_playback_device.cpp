#include "alsa_playback_device.h"
#include "qdebug.h"
#include "qthread.h"
#include "parameters.h"
#include "alsa_util.h"
#include "click_c.h"

alsa_playback_device::alsa_playback_device(QString device,  int channels, int rate, interface_c *interface,bool *success):deviceName(device),pInterface(interface)
{

    deviceDesc = getCardDescription(device,SND_PCM_STREAM_PLAYBACK);


    if (!alsa_open_device_playback(device)) return;


    alsa_init_playback(channels,rate,interface);

    alsa_set_hw_parameters_playback();

    alsa_set_sw_parameters_playback();

    alsa_begin_playback(main_buf_playback);

    *success = true;
}

bool alsa_playback_device::alsa_open_device_playback(QString device)
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

            qDebug()<<"cannot open audio device"<<device.toStdString().c_str()<<"for capture"<<snd_strerror (err);


            return false;
        }
    }

    return true;

}

void alsa_playback_device::alsa_init_playback(int channels,int rate,interface_c *interface)
{

    playback_channels = channels;
    playback_rate = rate;

    //todo fix this
    playback_frames = PLAYBACK_CHANNEL_WIDTH;
    playback_period_size = playback_frames*playback_channels;






    main_buf_playback = (playback_port_c**)malloc(channels*sizeof(playback_port_c));
    playback_buf = (short**)malloc(playback_channels*sizeof(short*));


    for(int i =0;i<channels;i++)
    {
        interface->playbackPortsCount++;
        main_buf_playback[i] = new playback_port_c(RINGBUFSIZE_PLAYBACK,interface->playbackPortsCount,deviceDesc,interface);
        playback_buf[i] = main_buf_playback[i]->consumer->buf;
    }





}

void alsa_playback_device::alsa_set_hw_parameters_playback(void)
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

void alsa_playback_device::alsa_set_sw_parameters_playback(void)
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

void alsa_playback_device::alsa_begin_playback(playback_port_c **port)
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

    consumer = new ConsumerDevicePlayback();
    consumer->playing = true;
    consumer->controler = this;
    consumer->port = port;
    consumer->start();





}

void alsa_playback_device::alsa_write_playback(playback_port_c **port)
{
    int err;





    for(int i = 0;i<playback_channels;i++)
    {


        port[i]->consumer->pullN(playback_frames);

    }






    if(pInterface->pClick)
    {



        for(int i = 0;i<playback_channels;i++)
        {
            if(port[i]->clickDataToPlay0>0)
            {

             /*   if(port[i]->nuTop)
                {
                    port[i]->nuTop = false;
                    qDebug()<<"port"<<i<<" : "<<port[i]->telapsed.elapsed();

                }
*/

            for(unsigned int j = 0;j<playback_frames;j++)
            {
                if(port[i]->clickDataToPlay0<1)
                    break;
                port[i]->consumer->buf[j]+=pInterface->pClick->bufClick0[pInterface->pClick->bufClick0_L-port[i]->clickDataToPlay0]/5;
                port[i]->clickDataToPlay0--;
            }

            }



            if(port[i]->clickDataToPlay1>0)
            {

              /*  if(port[i]->nuTop)
                {
                    port[i]->nuTop = false;
                    qDebug()<<"port"<<i<<" : "<<port[i]->telapsed.elapsed();

                }*/


            for(unsigned int j = 0;j<playback_frames;j++)
            {
                if(port[i]->clickDataToPlay1<1)
                    break;
                port[i]->consumer->buf[j]+=pInterface->pClick->bufClick1[pInterface->pClick->bufClick1_L-port[i]->clickDataToPlay1]/5;
                port[i]->clickDataToPlay1--;
            }

            }






        }


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


void alsa_playback_device::alsa_conf(void)
{
    snd_pcm_uframes_t  	buffer_size;
    snd_pcm_uframes_t period_size;
    snd_pcm_get_params 	( 	playback_handle,       &buffer_size, 	&period_size );

}

playback_port_c* alsa_playback_device::alsa_playback_port_by_num(int channel)
{


    return main_buf_playback[channel];
}

void alsa_playback_device::alsa_cleanup_playback()
{
    consumer->playing = false;



}

int ConsumerDevicePlayback::wait_for_poll(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count)
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

void ConsumerDevicePlayback::write_and_poll_loop(playback_port_c **port)
{
    struct pollfd *ufds;

    int err, count;
    count = snd_pcm_poll_descriptors_count (controler->playback_handle);
    if (count <= 0) {
        qDebug()<<"Invalid poll descriptors count\n";
        return ;
    }

    ufds = (struct pollfd*)malloc(sizeof(struct pollfd) * count);
    if (ufds == NULL) {
        qDebug()<<"No enough memory\n";
        return ;
    }
    if ((err = snd_pcm_poll_descriptors(controler->playback_handle, ufds, count)) < 0) {
        qDebug()<<"Unable to obtain poll descriptors for playback: "<<snd_strerror(err);
        return ;
    }





    while (playing) {
        err = wait_for_poll(controler->playback_handle, ufds, count);
        if (err < 0) {

            qDebug()<<"playback polling error"<<err;
            controler->pInterface->alsaDestroy();
            return;}
        else
        {
            break;
        }

    }


    snd_pcm_uframes_t avail;
    avail = snd_pcm_avail_update(controler->playback_handle);

    // qDebug()<<avail;
    while(avail >= controler->playback_frames)
    {
        controler->alsa_write_playback(port);
        avail = snd_pcm_avail_update(controler->playback_handle);
    }

}

void ConsumerDevicePlayback::run()
{

    while(playing)
    {
        write_and_poll_loop(port);
        QThread::usleep(PLAYBACK_READBUF_SLEEP);
    }


    snd_pcm_close(controler->playback_handle);
    free(controler->playback_buf);
    free(controler->main_buf_playback);
    controler->deleteLater();






}


