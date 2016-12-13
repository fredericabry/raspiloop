#include "alsa_util.h"
#include "qdebug.h"








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
            qDebug()<<"unable to open"<<name<<"(getcardlistlength)";
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
            qDebug()<<"unable to open"<<name<<"(getCardList)";
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
            CardName->append(QString(name)+","+QString::number(dev));
            card_desc =
                    (QString)//snd_ctl_card_info_get_id(info)

                    +snd_ctl_card_info_get_name(info)
                    +" - "
                    +snd_pcm_info_get_name(pcminfo)

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










QString getCardDescription(QString cardName,snd_pcm_stream_t stream)
{



    QString card_desc;


    snd_ctl_t *handle;
    int err;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);


    QStringList nameParts = cardName.split(",",QString::SkipEmptyParts);

    if(nameParts.size()!=2)
    {
        qDebug()<<"card name error";
        return "";

    }
    QString name = nameParts[0];
    QString id = nameParts[1];
    QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
    if (!re.exactMatch(id))
    {
        qDebug()<<"card name error";
        return "";

    }

    if((err = snd_ctl_open(&handle,name.toStdString().c_str(),0))<0)
    {
        qDebug()<<"cannot get card description, unable to open"<<name;
        return "";
    }


    if((err = snd_ctl_card_info(handle,info))<0)
    {
        qDebug()<<"control hardware info (%i): %s";
        snd_ctl_close(handle);
        return "";
    }


    snd_pcm_info_set_device(pcminfo,id.toInt());
    snd_pcm_info_set_subdevice(pcminfo,0);
    snd_pcm_info_set_stream(pcminfo,stream);

    if((err = snd_ctl_pcm_info(handle,pcminfo))<0)
    {
        if(err != -ENOENT) qDebug() << "control digital audio info";
        return "";

    }

    card_desc =
            (QString)
            +snd_ctl_card_info_get_name(info)
            +" - "
            +snd_pcm_info_get_name(pcminfo)
            ;



    return card_desc;

}




struct hw_info get_device_info(QString device_name,snd_pcm_stream_t stream) {
    snd_pcm_hw_params_t *hw_params;
    int err;
    snd_pcm_t *handle;
    unsigned int max;
    unsigned int min;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;


    struct hw_info info;


    info.name = device_name;

    // stream = SND_PCM_STREAM_PLAYBACK;



    if ((err = snd_pcm_open (&handle, device_name.toStdString().c_str(), stream, 0)) < 0) {

        info.max_port = 0;
        info.min_port = 0;
        return info;
    }



    //  info.longname = (QString)snd_pcm_name(handle);
    info.longname = "";


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

    info.max_port = max;



    if ((err = snd_pcm_hw_params_get_channels_min(hw_params, &min)) < 0) {
        fprintf (stderr, "cannot get channel info  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    info.min_port = min;



    if ((err = snd_pcm_hw_params_get_rate_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get min rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    info.min_rate = val;


    if ((err = snd_pcm_hw_params_get_rate_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get max rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    info.max_rate = val;

    if ((err = snd_pcm_hw_params_get_period_time_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get min period time  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }

    info.min_period_time = val;

    if ((err = snd_pcm_hw_params_get_period_time_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot  get max period time  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("max period time %d usecs\n", val);
    info.max_period_time = val;

    if ((err = snd_pcm_hw_params_get_period_size_min(hw_params, &frames, &dir)) < 0) {
        fprintf (stderr, "cannot  get min period size  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min period size in frames %d\n", frames);
    info.min_period_size = frames  ;

    if ((err = snd_pcm_hw_params_get_period_size_max(hw_params, &frames, &dir)) < 0) {
        fprintf (stderr, "cannot  get max period size (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("max period size in frames %d\n", frames);
    info.max_period_size = frames;

    if ((err = snd_pcm_hw_params_get_periods_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot  get min periods  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("min periods per buffer %d\n", val);
    info.min_period_per_buffer= val;

    if ((err = snd_pcm_hw_params_get_periods_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot  get min periods (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("max periods per buffer %d\n", val);
    info.max_period_per_buffer= val;

    if ((err = snd_pcm_hw_params_get_buffer_time_min(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get min buffer time (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min buffer time %d usecs\n", val);
    info.min_buffer_time = val;
    if ((err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &val, &dir)) < 0) {
        fprintf (stderr, "cannot get max buffer time  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //  printf("max buffer time %d usec  buf+="min buffer size in frames " + n2s(frames) + "\n";s\n", val);
    info.max_buffer_time = val;
    if ((err = snd_pcm_hw_params_get_buffer_size_min(hw_params, &frames)) < 0) {
        fprintf (stderr, "cannot get min buffer size (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    //printf("min buffer size in frames %d\n", frames);
    info.min_buffer_size = frames;


    if ((err = snd_pcm_hw_params_get_buffer_size_max(hw_params, &frames)) < 0) {
        fprintf (stderr, "cannot get max buffer size  (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    // printf("max buffer size in frames %d\n", frames);
    info.max_buffer_size = frames;



    snd_pcm_close(handle);




    return info;

}



















//midi




void list_subdevice_info(snd_ctl_t *ctl, int card, int device) {
   snd_rawmidi_info_t *info;
   const char *name;
   const char *sub_name;
   int subs, subs_in, subs_out;
   int sub, in, out;
   int status;

   snd_rawmidi_info_alloca(&info);

   snd_rawmidi_info_set_device(info, device);

   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
   snd_ctl_rawmidi_info(ctl, info);
   subs_in = snd_rawmidi_info_get_subdevices_count(info);


   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
   snd_ctl_rawmidi_info(ctl, info);
   subs_out = snd_rawmidi_info_get_subdevices_count(info);

   subs = subs_in > subs_out ? subs_in : subs_out;

   sub = 0;
   in = out = 0;
   if ((status = is_output(ctl, card, device, sub)) < 0) {
      qDebug()<<"cannot get rawmidi information"<<card<<device<<snd_strerror(status);
      return;
   } else if (status)
      out = 1;

   if (status == 0) {
      if ((status = is_input(ctl, card, device, sub)) < 0) {
         qDebug()<<"cannot get rawmidi information"<<card<<device<<snd_strerror(status);
         return;
      }
   } else if (status)
      in = 1;

   if (status == 0)
      return;

   name = snd_rawmidi_info_get_name(info);
   sub_name = snd_rawmidi_info_get_subdevice_name(info);
   if (sub_name[0] == '\0') {
      if (subs == 1) {
       /*  printf("%c%c  hw:%d,%d    %s\n",
                in  ? 'I' : ' ',
                out ? 'O' : ' ',
                card, device, name);*/


         qDebug()<<card<<device<<name<<subs;
         if(in)qDebug()<<"input";
         if(out)qDebug()<<"output";

      } else
       /*  printf("%c%c  hw:%d,%d    %s (%d subdevices)\n",
                in  ? 'I' : ' ',
                out ? 'O' : ' ',
                card, device, name, subs);*/
          qDebug()<<card<<device<<name<<subs;
          if(in)qDebug()<<"input";
          if(out)qDebug()<<"output";


   } else {
      sub = 0;
      for (;;) {
       /*  printf("%c%c  hw:%d,%d,%d  %s\n",
                in ? 'I' : ' ', out ? 'O' : ' ',
                card, device, sub, sub_name);*/
          qDebug()<<card<<device<<name<<subs;
          if(in)qDebug()<<"input";
          if(out)qDebug()<<"output";


         if (++sub >= subs)
            break;

         in = is_input(ctl, card, device, sub);
         out = is_output(ctl, card, device, sub);
         snd_rawmidi_info_set_subdevice(info, sub);
         if (out) {
            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);
            if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0) {
               qDebug()<<"cannot get rawmidi information"<<card<< device<< sub<< snd_strerror(status);
               break;
            }
         } else {
            snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
            if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0) {
               qDebug()<<"cannot get rawmidi information"<<card<< device<< sub<< snd_strerror(status);
               break;
            }
         }
         sub_name = snd_rawmidi_info_get_subdevice_name(info);

      }
   }
}






//////////////////////////////
//
// is_input -- returns true if specified card/device/sub can output MIDI data.
//

int is_input(snd_ctl_t *ctl, int card, int device, int sub) {
   snd_rawmidi_info_t *info;
   int status;

   snd_rawmidi_info_alloca(&info);
   snd_rawmidi_info_set_device(info, device);
   snd_rawmidi_info_set_subdevice(info, sub);
   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);

   if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
      return status;
   } else if (status == 0) {
      return 1;
   }

   return 0;
}



//////////////////////////////
//
// is_output -- returns true if specified card/device/sub can output MIDI data.
//

int is_output(snd_ctl_t *ctl, int card, int device, int sub) {
   snd_rawmidi_info_t *info;
   int status;

   snd_rawmidi_info_alloca(&info);
   snd_rawmidi_info_set_device(info, device);
   snd_rawmidi_info_set_subdevice(info, sub);
   snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_OUTPUT);

   if ((status = snd_ctl_rawmidi_info(ctl, info)) < 0 && status != -ENXIO) {
      return status;
   } else if (status == 0) {
      return 1;
   }

   return 0;
}




void error(const char *format, ...) {
   va_list ap;
   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);
   qDebug()<<stderr;
}










