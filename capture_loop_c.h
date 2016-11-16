#ifndef CAPTURE_LOOP_C_H
#define CAPTURE_LOOP_C_H

#include <QMainWindow>
#include <sndfile.h>
#include "parameters.h"
#include "qstring.h"
#include "qthread.h"
#include "qelapsedtimer.h"
#include "capture_port_c.h"



class capture_loop_c;



class captureLoopConsumer:public QThread
{
    Q_OBJECT

    void run() Q_DECL_OVERRIDE;
public:
    capture_loop_c* port;

    QElapsedTimer telapsed;

public slots:
    void update(void);


};


class capture_loop_c:public QObject
{
    Q_OBJECT
public:
    capture_loop_c(const QString id, const QString filename, capture_port_c *pPort, const int rate, const unsigned long bufsize, long length);
    ~capture_loop_c();
    void destroyLoop(void);
    void openfile(QString filename);
    void closefile(void);
    int pullN(unsigned long N);
    unsigned long length(void);
    const QString id;
    const QString filename;
    capture_port_c *pPort;
    const int rate;
    const unsigned long bufsize;
    SNDFILE *soundfile;//file handler
    short *buffile; //buffer used for file writting
    unsigned long tail;//last value read in the associated ringbuffer
    captureLoopConsumer *consumer;
    bool recording;
    QString filedir;

    bool stop;//true if a length is specified and therefore the recording has to self stop
    long framesToPlay;//values left to be recorded



};

#endif // CAPTURE_LOOP_C_H





