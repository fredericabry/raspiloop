#ifndef CAPTURE_PORT_C_H
#define CAPTURE_PORT_C_H



#include <sndfile.h>
#include <alsa/asoundlib.h>
#include <QMainWindow>


class capture_port_c : public QObject
{
    Q_OBJECT

public:
    capture_port_c(const int maxlength,const int bufsize,const int rate);
    ~capture_port_c();


    int const maxlength;
    int const bufsize;
    int const rate;
    SNDFILE *soundfile;

    bool recording;

    short *ringbuf;//big buffer for circular storage
    short *buf;//small buffer for transfert


    int oldest;



    void fill(int channel);
    int length();
    void pushN(int N);
    void pullN(int pos, int N);
    void openfile(QString filename);
    void closefile();
    void destroyport();
    void startrecord();
    void stoprecord();

};


#endif // CAPTURE_PORT_C_H
