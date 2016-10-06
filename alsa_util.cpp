#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include <stdbool.h>


#include <QFile>
#include <QTextStream>

#include <qdebug.h>



snd_pcm_t *capture_handle;
SNDFILE *sf_record;
short *buf_record;
snd_pcm_uframes_t frames_record,bufsize_record;
MainWindow *parent_record;
int nbr_chan_record ;
int rate_record;
long frame_count_record;


SNDFILE *sf_play;
MainWindow *parent_play;
snd_pcm_t *play_handle;
snd_pcm_uframes_t frames_play,bufsize_play;
short *buf_play;
int rate_play;
int nbr_chan_play;
long frame_count_play;
long available_frame_play;

#define FORM_FILE SND_PCM_FORMAT_S16_LE
















/*
 ************************
Playback functions
*************************
************************* */


void alsa_play(QString device, int channels, int rate, long length, QString filename,MainWindow *pt)
{



    parent_play= pt;
    init_play(channels,rate,length);
    open_file_play(filename);
    set_hw_parameters_play(device);
    set_sw_parameters_play();
    start_play();


}





void init_play(int channels,int rate, long length)
{

    parent_play->Afficher("début playback\n");

    nbr_chan_play = channels;
    rate_play = rate;




}


void open_file_play(QString filename)
{
    SF_INFO sf_info;

    sf_info.format = 0;
    if ((sf_play = sf_open (filename.toStdString().c_str(), SFM_READ, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile for output %s\n", errstr);

        exit (1);
    }







    int short_mask = SF_FORMAT_PCM_16;

    if(sf_info.format != (SF_FORMAT_WAV|short_mask))
    {
        parent_play->Afficher("format de fichier incorrect\n");
        return;
    }

    if(sf_info.samplerate != rate_play)
    {
        parent_play->Afficher("soundfile rate incorrect\n");
        return;
    }
    if(sf_info.channels != nbr_chan_play)
    {
        parent_play->Afficher("chan nbr incorrect\n");
        return;
    }


    available_frame_play = sf_info.frames;






}

void set_hw_parameters_play(QString device)
{

    int err;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate = rate_record;


    if ((err = snd_pcm_open (&play_handle,device.toStdString().c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {

        if(err == -EBUSY)
        {



        }
        else
        {


        fprintf (stderr, "cannot open audio device %s (%s)\n",
                 device.toStdString().c_str(),
                 snd_strerror (err));

        exit (1);
    }
EPIPE

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (play_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (play_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_hw_params_set_format (play_handle, hw_params, FORM_FILE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near (play_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_hw_params_set_channels (play_handle, hw_params, nbr_chan_play)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    //todo fix this
    frames_play = 128*nbr_chan_play;
    bufsize_play = 5*frames_play;


    snd_pcm_hw_params_set_period_size_near(play_handle, hw_params, &frames_play,0);


    buf_play =  (short*)malloc(bufsize_play*sizeof(short));


    if ((err = snd_pcm_hw_params (play_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }




    snd_pcm_hw_params_free (hw_params);


}

//software parameters
void set_sw_parameters_play(void)
{


    int err;

    snd_pcm_sw_params_t *swparams;

    snd_pcm_sw_params_alloca(&swparams);

    err = snd_pcm_sw_params_current(play_handle, swparams);
    if (err < 0) {
        fprintf(stderr, "Broken configuration for this PCM: no configurations available\n");
        exit(1);
    }


    //envoie les données quand il y a plus de *threshold* données dans le buf de la carte
    err = snd_pcm_sw_params_set_start_threshold(play_handle, swparams,  frames_play);
    if (err < 0) {
        printf("Unable to set start threshold: %s\n", snd_strerror(err));
        exit(1);
    }


    err = snd_pcm_sw_params_set_avail_min(play_handle, swparams, frames_play);
    if (err < 0) {
        printf("Unable to set avail min: %s\n", snd_strerror(err));
        exit(1);
    }

    if (snd_pcm_sw_params(play_handle, swparams) < 0) {
        fprintf(stderr, "unable to install sw params:\n");
        exit(1);
    }


}

void start_play(void)
{
    int nread,err;


    if ((err = snd_pcm_prepare (play_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }







    snd_async_handler_t *pcm_callback;
    snd_async_add_pcm_handler(&pcm_callback,play_handle,async_callback_play,NULL);


    for(int i = 0; i < 3 ; i ++)
    {
        if((nread = sf_readf_short(sf_play,buf_play,frames_play))>0)
        {

            if ((err = snd_pcm_writei (play_handle, buf_play, nread))!=nread) {
                exit(0);
                qDebug()<<"play on audio interface failed ";
            }
            else
                available_frame_play -= nread;

        }
    }









}


void async_callback_play(snd_async_handler_t *ahandler)
{


    int nread,err;
    snd_pcm_sframes_t avail;


    snd_pcm_t *play_handle = snd_async_handler_get_pcm(ahandler);



    avail = snd_pcm_avail_update(play_handle);

    while(avail >= frames_play)
    {
        if((nread = sf_readf_short(sf_play,buf_play,frames_play))>0)
        {

            if ((err = snd_pcm_writei (play_handle, buf_play, nread))!=nread) {
                qDebug()<<"play on audio interface failed ";
                exit(0);
            }
            else
            {

                available_frame_play -= nread;    /*
  int i = 0;
while(i<1000)
    {
    if ((nwrite = snd_pcm_writei (play_handle, buf_play, frames_play))<0) {
        qDebug()<<"play on audio interface failed ";
        // recover
        snd_pcm_prepare(play_handle);
    } else {
        //if (sf_write_raw (sf_record, buf_record, sizeof(short)* nread*nbr_chan_record) != sizeof(short)* nread*nbr_chan_record)   qDebug()<< "cannot write sndfile";
    }
    i++;
    }
*/
            }

            avail = snd_pcm_avail_update(play_handle);
        }


        else
        {


            stop_play();
                       break;
        }


    }




}

void stop_play(void)
{
   // parent_play->Afficher("fin playback\n");


if(snd_pcm_state(play_handle) == SND_PCM_STATE_XRUN)
{
snd_pcm_drain(play_handle);
snd_pcm_unlink(play_handle);
snd_pcm_close(play_handle);
}

  //
//   qDebug()<<snd_pcm_state(play_handle);

}
/*



/*
 ************************
Capture functions
*************************
************************* */



void stop_record(void)
{
    parent_record->Afficher("fin enregistrement\n");
    snd_pcm_drop(capture_handle);
}


void init_record(int channels,int rate, long length)
{

    parent_record->Afficher("début enregistrement\n");

    nbr_chan_record = channels;
    rate_record = rate;

    frame_count_record = ((length/10)*(rate/100)*channels); //nbr of frames expected to record "length ms"


    //parent_record-> Afficher(n2s(frame_count));

}


void alsa_record(QString device, int channels, int rate, long length, QString filename,MainWindow *pt)
{


    if((length > 360000) || (length<=0))
    {
        pt->Afficher("temps enregistrement incorrect\n");
        return;
    }

    parent_record= pt;
    init_record(channels,rate,length);
    open_file_record(filename);
    set_hw_parameters_record(device);
    set_sw_parameters_record();
    start_record();


}


void start_record(void)
{
    int nread,err;


    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((nread = snd_pcm_readi (capture_handle, buf_record, frames_record))<0) {
        qDebug()<<"read from audio interface failed ";
        // recover
        snd_pcm_prepare(capture_handle);
    } else {
        if (sf_write_raw (sf_record, buf_record, sizeof(short)* nread*nbr_chan_record) != sizeof(short)* nread*nbr_chan_record)   qDebug()<< "cannot write sndfile";
    }


    frame_count_record -= nread*nbr_chan_record;

    snd_async_handler_t *pcm_callback;
    snd_async_add_pcm_handler(&pcm_callback,capture_handle,async_callback_record,NULL);

}


//software parameters
void set_sw_parameters_record(void)
{


    int err;

    snd_pcm_sw_params_t *swparams;

    snd_pcm_sw_params_alloca(&swparams);

    err = snd_pcm_sw_params_current(capture_handle, swparams);
    if (err < 0) {
        fprintf(stderr, "Broken configuration for this PCM: no configurations available\n");
        exit(1);
    }



    err = snd_pcm_sw_params_set_start_threshold(capture_handle, swparams,  frames_record);
    if (err < 0) {
        printf("Unable to set start threshold: %s\n", snd_strerror(err));
        exit(1);
    }
    err = snd_pcm_sw_params_set_avail_min(capture_handle, swparams, frames_record);
    if (err < 0) {
        printf("Unable to set avail min: %s\n", snd_strerror(err));
        exit(1);
    }

    if (snd_pcm_sw_params(capture_handle, swparams) < 0) {
        fprintf(stderr, "unable to install sw params:\n");
        exit(1);
    }


}


//hardware parameters
void set_hw_parameters_record(QString device)
{

    int err;
    snd_pcm_hw_params_t *hw_params;
    unsigned int rate = rate_record;




    if ((err = snd_pcm_open (&capture_handle,device.toStdString().c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
                 device.toStdString().c_str(),
                 snd_strerror (err));

        exit (1);
    }


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

    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, FORM_FILE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }






    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, nbr_chan_record)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }



    frames_record = 128*nbr_chan_record;
    bufsize_record = 5*frames_record;


    snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, &frames_record,0);


    buf_record =  (short*)malloc(bufsize_record*sizeof(short));


    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }




    snd_pcm_hw_params_free (hw_params);


}


//Setup the wave file used to record
void open_file_record(QString filename)
{

    SF_INFO sf_info;
    int short_mask;

    sf_info.samplerate = rate_record;
    sf_info.channels = nbr_chan_record;

    short_mask = SF_FORMAT_PCM_16;

    sf_info.format = SF_FORMAT_WAV|short_mask;

    if ((sf_record = sf_open (filename.toStdString().c_str(), SFM_WRITE, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile \"%s\" for output (%s)\n", "test.wav", errstr);

        exit (1);
    }


}


void async_callback_record(snd_async_handler_t *ahandler)
{
    snd_pcm_t *capture_handle = snd_async_handler_get_pcm(ahandler);
    snd_pcm_sframes_t avail;
    int nread;

    avail = snd_pcm_avail_update(capture_handle);

    while(avail >= (snd_pcm_sframes_t)frames_record)
    {

        if(frame_count_record <= 0)
        {

            stop_record();
            return;

        }
        else if(frame_count_record <= (snd_pcm_sframes_t)frames_record)
        {

            if ((nread = snd_pcm_readi (capture_handle, buf_record, frame_count_record))<0) {
                qDebug()<<"read from audio interface failed ";
                snd_pcm_prepare(capture_handle);
            } else
            {
                if (sf_write_raw (sf_record, buf_record, sizeof(short)* nread*nbr_chan_record) != sizeof(short)* nread*nbr_chan_record)   qDebug()<< "cannot write sndfile";
                frame_count_record -= nread*nbr_chan_record;

            }

            stop_record();
            return;


        }
        else
        {

            if ((nread = snd_pcm_readi (capture_handle, buf_record, frames_record))<0) {
                qDebug()<<"read from audio interface failed ";

                // recover
                snd_pcm_prepare(capture_handle);
            } else {



                if (sf_write_raw (sf_record, buf_record, sizeof(short)* nread*nbr_chan_record) != sizeof(short)* nread*nbr_chan_record)   qDebug()<< "cannot write sndfile";
                frame_count_record -= nread*nbr_chan_record;



            }
            avail = snd_pcm_avail_update(capture_handle);

        }

    }

}


QStringList pcm_list(snd_pcm_stream_t stream)
{
    QString device;
    QStringList devicelist;

    void **hints, **n;
    char *name, *descr, *descr1, *io;
    const char *filter;

    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return devicelist;

    n = hints;
    filter = stream == SND_PCM_STREAM_CAPTURE ? "Input" : "Output";

    while (*n != NULL) {
        name = snd_device_name_get_hint(*n, "NAME");
        descr = snd_device_name_get_hint(*n, "DESC");
        io = snd_device_name_get_hint(*n, "IOID");
        if (io != NULL && strcmp(io, filter) == 0)
            goto __end;




        device = name;

        // printf("%s\n", name);

        if ((descr1 = descr) != NULL) {

            device+= descr1;

            device+="\n------------------------------\n";
            devicelist.append(device);

        }
__end:
        if (name != NULL)
            free(name);
        if (descr != NULL)
            free(descr);
        if (io != NULL)
            free(io);
        n++;
    }
    snd_device_name_free_hint(hints);
    return devicelist;
}


int getCardListLength(snd_pcm_stream_t stream)
{

    snd_ctl_t *handle;
    int card,err,dev,count;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    card = -1;
    if( (snd_card_next(&card)<0 ) || (card<0))
    {
        qDebug()<<"No sound cards\n"  ;
        return 0;
    }

    count = 0;

    while (card >= 0)
    {


        char name[32];
        sprintf(name,"hw:%d",card);
        if((err = snd_ctl_open(&handle,name,0))<0)
        {
            qDebug()<<"control open (%i): %s";
            goto next_card;
        }


        if((err = snd_ctl_card_info(handle,info))<0)
        {
            qDebug()<<"control hardware info (%i): %s";
            snd_ctl_close(handle);
            goto next_card;
        }



        dev=-1;
        while(1)
        {

            //unsigned int count;
            if(snd_ctl_pcm_next_device(handle,&dev)<0)
                qDebug()<<"snd_ctl_pcm_next_device";
            if(dev<0)
                break;
            snd_pcm_info_set_device(pcminfo,dev);
            snd_pcm_info_set_subdevice(pcminfo,0);
            snd_pcm_info_set_stream(pcminfo,stream);

            if((err = snd_ctl_pcm_info(handle,pcminfo))<0)
            {
                if(err != -ENOENT) qDebug() << "control digital audio info";
                continue;

            }

            count++;



        }




next_card:
        if(snd_card_next(&card) < 0)
        {
            qDebug()<<"snd_card_next";
            break;
        }





    }




    return count;

}


void getCardList(snd_pcm_stream_t stream,QStringList *CardName,QStringList *CardInfos)
{



    QString card_desc;



    snd_ctl_t *handle;
    int card,err,dev;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);


    CardName->clear();
    CardInfos->clear();


    card = -1;
    if( (snd_card_next(&card)<0 ) || (card<0))
    {
        qDebug()<<"No sound cards\n"  ;
        return ;
    }



    while (card >= 0)
    {


        char name[32];
        sprintf(name,"hw:%d",card);
        if((err = snd_ctl_open(&handle,name,0))<0)
        {
            qDebug()<<"control open (%i): %s";
            goto next_card;
        }


        if((err = snd_ctl_card_info(handle,info))<0)
        {
            qDebug()<<"control hardware info (%i): %s";
            snd_ctl_close(handle);
            goto next_card;
        }



        dev=-1;
        while(1)
        {

            //unsigned int count;
            if(snd_ctl_pcm_next_device(handle,&dev)<0)
                qDebug()<<"snd_ctl_pcm_next_device";
            if(dev<0)
                break;
            snd_pcm_info_set_device(pcminfo,dev);
            snd_pcm_info_set_subdevice(pcminfo,0);
            snd_pcm_info_set_stream(pcminfo,stream);

            if((err = snd_ctl_pcm_info(handle,pcminfo))<0)
            {
                if(err != -ENOENT) qDebug() << "control digital audio info";
                continue;

            }

            //count = snd_pcm_info_get_subdevices_count(pcminfo);
            CardName->append(name);
            card_desc =
                    (QString)snd_ctl_card_info_get_id(info)
                    +" ["
                    +snd_ctl_card_info_get_name(info)
                    +"], "
                    //+"device "
                    //+QString::number(dev)
                    //+": "
                    +snd_pcm_info_get_id(pcminfo)
                    +"["
                    +snd_pcm_info_get_name(pcminfo)
                    +"]"
                    //+"\nSubdevices: "
                    //+QString::number(snd_pcm_info_get_subdevices_avail(pcminfo))
                    //+"\\"
                    //+n2s(count)
                    //+"\n"
                    ;







            CardInfos->append(card_desc);

        }









next_card:
        if(snd_card_next(&card) < 0)
        {
            qDebug()<<"snd_card_next";
            break;
        }





    }



}


QString info(QString qdev_name, snd_pcm_stream_t stream) {
    snd_pcm_hw_params_t *hw_params;
    int err;
    snd_pcm_t *handle;
    unsigned int max;
    unsigned int min;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    const char *dev_name;
    dev_name = qdev_name.toStdString().c_str();

    QString buf;

    if ((err = snd_pcm_open (&handle, dev_name, stream, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
                 dev_name,
                 snd_strerror (err));
        return buf;
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_get_channels_max(hw_params, &max)) < 0) {
        fprintf (stderr, "cannot  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    buf+="max channels" + n2s(max) + "\n";

    if ((err = snd_pcm_hw_params_get_channels_min(hw_params, &min)) < 0) {
        fprintf (stderr, "cannot get channel info  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min channels %d\n", min);
    buf+="min channels" + n2s(min) + "\n";

    /*
  if ((err = snd_pcm_hw_params_get_sbits(hw_params)) < 0) {
      fprintf (stderr, "cannot get bits info  (%s)\n",
           snd_strerror (err));
      exit (1);
  }
  printf("bits %d\n", err);
  */

    if ((err = snd_pcm_hw_params_get_rate_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get min rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min rate %d hz\n", val);
    buf+="min rate " + n2s(val) + "Hz\n";

    if ((err = snd_pcm_hw_params_get_rate_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get max rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("max rate %d hz\n", val);
    buf+="max rate " + n2s(val) + "Hz\n";


    if ((err = snd_pcm_hw_params_get_period_time_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get min period time  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min period time %d usecs\n", val);
    buf+="min period time " + n2s(val) + "us\n";


    if ((err = snd_pcm_hw_params_get_period_time_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot  get max period time  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("max period time %d usecs\n", val);
    buf+="max period time " + n2s(val) + "us\n";

    if ((err = snd_pcm_hw_params_get_period_size_min(hw_params, &frames, &dir)) < 0) {
        fprintf (stderr, "cannot  get min period size  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min period size in frames %d\n", frames);
    buf+="min period size in frames " + n2s(frames) + "\n";

    if ((err = snd_pcm_hw_params_get_period_size_max(hw_params, &frames, &dir)) < 0) {
        fprintf (stderr, "cannot  get max period size (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("max period size in frames %d\n", frames);
    buf+="max period size in frames " + n2s(frames) + "\n";

    if ((err = snd_pcm_hw_params_get_periods_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot  get min periods  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("min periods per buffer %d\n", val);
    buf+="min period per buffer " + n2s(val) + "\n";

    if ((err = snd_pcm_hw_params_get_periods_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot  get min periods (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("max periods per buffer %d\n", val);
    buf+="max period per buffer " + n2s(val) + "\n";

    if ((err = snd_pcm_hw_params_get_buffer_time_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get min buffer time (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min buffer time %d usecs\n", val);
    buf+="min buffer time " + n2s(val) + " usecs\n";
    if ((err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get max buffer time  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //  printf("max buffer time %d usec  buf+="min buffer size in frames " + n2s(frames) + "\n";s\n", val);
    buf+="max buffer time " + n2s(val) + " usecs\n";
    if ((err = snd_pcm_hw_params_get_buffer_size_min(hw_params, &frames)) < 0) {
        fprintf (stderr, "cannot get min buffer size (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min buffer size in frames %d\n", frames);
    buf+="min buffer size in frames " + n2s(frames) + "\n";


    if ((err = snd_pcm_hw_params_get_buffer_size_max(hw_params, &frames)) < 0) {
        fprintf (stderr, "cannot get max buffer size  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("max buffer size in frames %d\n", frames);
    buf+="max buffer size in frames " + n2s(frames) + "\n";

    snd_pcm_close(handle);
    return buf;

}







