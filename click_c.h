#ifndef CLICK_C_H
#define CLICK_C_H
#include <QMainWindow>
#include "qtimer.h"
#include "qelapsedtimer.h"
#include "playback_loop_c.h"

class playback_port_c;
class playback_loop_c;

class click_c:public QObject
{
    Q_OBJECT
private:
        int tempo;

public:
    click_c(int tempo, std::vector<playback_port_c *> pPorts, status_t status, interface_c *interface, MainWindow *parent);

    std::vector<playback_port_c*> pPorts;
    int status;
    double getBeat(void);//time elapsed since the begining of bar in beats
    double getTime(void);//time elapsed since the begining of bar in s;
    QTimer timer;
    playback_loop_c *pDing;
    QElapsedTimer *t1;
    void stopstart(void);
    interface_c *interface;
    MainWindow *parent;


    int getTempo(void);
    bool isActive(void);
    void stop(void);
    void start(void);



private:
    int beat;



public slots:
    void tick();
    void setTempo(int);
    void clickUp(void);
    void clickDown(void);

signals:
    void firstBeat(void);//signal the first beat of the bar


};

#endif // CLICK_C_H
