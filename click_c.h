#ifndef CLICK_C_H
#define CLICK_C_H
#include <QMainWindow>
#include "qtimer.h"
#include "qelapsedtimer.h"


class playback_port_c;
class playback_loop_c;

class click_c:public QObject
{
    Q_OBJECT

public:
    click_c(int tempo, playback_port_c *pPort,int status);
    int tempo;
    int status;

    QTimer timer;
    playback_port_c *pPort;
    playback_loop_c *pDing;
    QElapsedTimer *t1;
public slots:
    void tick();


};

#endif // CLICK_C_H
