#ifndef ALSA_MIDI_H
#define ALSA_MIDI_H

#include "qstringlist.h"
#include <alsa/asoundlib.h>
#include "qobject.h"
#include "qthread.h"

#define MIDI_MAX_LENGTH 7 //maximum length of a midi sequence used for control

#define MIDI_ACTIVE_SENSING 254



void alsa_findMidiDevices(snd_rawmidi_stream_t stream, QStringList *longNames, QStringList *names);


class alsa_midi_capture_device;
class interface_c;

class alsa_midi_capture_device_consumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;

public:
    bool active;
    alsa_midi_capture_device *controler;

};





class alsa_midi_capture_device:public QObject
{
    Q_OBJECT

public:
    alsa_midi_capture_device(QString device,interface_c *pInterface, bool *success);
    ~alsa_midi_capture_device();
    void cleanup();
    bool alsa_openMidiDevice(QString deviceName);
    void alsa_midi_start_capture();
    snd_rawmidi_t *handle;
    alsa_midi_capture_device_consumer * consumer;

private:
    interface_c *pInterface;

signals:
    void sendMidiMsg(QString midiCode);

};


#endif // ALSA_MIDI_H
