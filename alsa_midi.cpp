#include "alsa_midi.h"
#include "alsa_util.h"
#include "qdebug.h"

#include "interface.h"

















void alsa_findMidiDevices(snd_rawmidi_stream_t stream,QStringList *longNames,QStringList *names)
{
    int status;
    int count;
    QString name,cardname,midiname;



    snd_rawmidi_info_t *info;

    int card = -1;  // use -1 to prime the pump of iterating through card list

    if ((status = snd_card_next(&card)) < 0) {
        qDebug()<<"cannot determine card number:"<< snd_strerror(status);
        return;
    }

    if (card < 0) {
        qDebug()<<"no sound cards found";
        return;
    }


    while (card >= 0) {


        snd_ctl_t *handle;

        int device = -1;
        int status;
        name = "hw:"+QString::number(card);
        if ((status = snd_ctl_open(&handle, name.toStdString().c_str(), 0)) < 0) {
            qDebug()<<"cannot open control for card"<< card<< snd_strerror(status);
            return;
        }

        do {
            status = snd_ctl_rawmidi_next_device(handle, &device);
            if (status < 0) {
                qDebug()<<"cannot determine device number: "<<snd_strerror(status);
                break;
            }

            if (device >= 0) {


                snd_rawmidi_info_alloca(&info);

                snd_rawmidi_info_set_device(info, device);


                snd_rawmidi_info_set_stream(info, stream);
                snd_ctl_rawmidi_info(handle, info);
                count = snd_rawmidi_info_get_subdevices_count(info);


                cardname = snd_rawmidi_info_get_name(info);
                midiname = snd_rawmidi_info_get_subdevice_name(info);
                if(count==1)
                {

                    (*longNames).append(midiname);
                    (*names).append(name+","+QString::number(device));
                }
                else
                {
                    for(int i = 0;i<count-1;i++)
                    {
                        (*longNames).append(midiname);
                        (*names).append(name+","+QString::number(device)+","+QString::number(i));
                    }
                }


            }
        } while (device >= 0);
        snd_ctl_close(handle);


        if ((status = snd_card_next(&card)) < 0) {

            qDebug()<<"cannot determine card number:"<<snd_strerror(status);
            break;
        }
    }


}


QString midiInterpreter(unsigned char c)
{
    QStringList noteList;
    noteList <<"C"<<"C#"<<"D"<<"D#"<<"E"<<"F"<<"F#"<<"G"<<"G#"<<"A"<<"A#"<<"B";


    QString txt ;

    int cmd = c;

    if(cmd ==MIDI_ACTIVE_SENSING) return txt;

    if((cmd>=0)&&(cmd<=127))
    {
        int octave = cmd/12;
        int note = cmd%12;

        txt = "note "+noteList.at(note)+QString::number(octave);
    }
    else if((cmd>=128)&&(cmd<=143))
    {
        txt = "NOff ch"+QString::number(cmd-127);
    }
    else if((cmd>=144)&&(cmd<=159))
    {
        txt = "NOn ch"+QString::number(cmd-143);
    }
    else if((cmd>=160)&&(cmd<=175))
    {
        txt = "PA ch"+QString::number(cmd-159);
    }
    else if((cmd>=176)&&(cmd<=191))
    {
        txt = "CC ch"+QString::number(cmd-175);
    }
    else if((cmd>=192)&&(cmd<=207))
    {
        txt = "PC ch"+QString::number(cmd-191);
    }
    else if((cmd>=208)&&(cmd<=223))
    {
        txt = "CA ch"+QString::number(cmd-207);
    }
    else if((cmd>=224)&&(cmd<=239))
    {
        txt = "PWR ch"+QString::number(cmd-223);
    }
    else if(cmd==248) txt="clock";
    else if(cmd==249) txt="undef249";
    else if(cmd==250) txt="start";
    else if(cmd==251) txt="continue";
    else if(cmd==252) txt="stop";
    else if(cmd==253) txt="undef253";
    //else if(cmd==254) txt="active sensing";
    else if(cmd == 255) txt="reset";
    else
        txt = QString::number(cmd);



    return txt;

}




alsa_midi_capture_device::alsa_midi_capture_device(QString device, interface_c *pInterface, bool *success):pInterface(pInterface)
{
    if(!alsa_openMidiDevice(device)) return;

    *success = true;
    connect(this,SIGNAL(sendMidiMsg(QString)),pInterface,SIGNAL(getMidiMsg(QString)));
    alsa_midi_start_capture();

}

bool alsa_midi_capture_device::alsa_openMidiDevice(QString deviceName)
{
    int err;
    if((err = snd_rawmidi_open(&handle,NULL,deviceName.toStdString().c_str(),0)<0))
    {


        if(err == -EBUSY)
        {
            qDebug()<<"Device " + deviceName + " busy";
            return false;
        }
        else
        {
            qDebug()<<"cannot open audio device"<<deviceName<<"for capture"<<snd_strerror (err);
            return false;
        }

    }
    return true;

}

void alsa_midi_capture_device::alsa_midi_start_capture()
{



    consumer = new alsa_midi_capture_device_consumer();

    consumer->controler = this;

    consumer->active = true;
    consumer->start();

}

alsa_midi_capture_device:: ~alsa_midi_capture_device()
{


}

void alsa_midi_capture_device::cleanup()
{
    consumer->active = false;

}


void alsa_midi_capture_device_consumer::run()
{
    unsigned char ch;

    while(active)
    {

        snd_rawmidi_read(controler->handle,&ch,1);
        if(ch)
        {

            QString txt = midiInterpreter(ch);
            if(txt.size()>0) qDebug()<<txt;


        }

        QThread::usleep(1000);
    }



    controler->deleteLater();

}

