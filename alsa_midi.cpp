#include "alsa_midi.h"
#include "alsa_util.h"
#include "qdebug.h"



















void alsa_findMidiDevices()
{
    int status;

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
          QString name;
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

                list_subdevice_info(handle, card, device);


             }
          } while (device >= 0);
          snd_ctl_close(handle);


         if ((status = snd_card_next(&card)) < 0) {

            qDebug()<<"cannot determine card number:"<<snd_strerror(status);
            break;
         }
      }













}
