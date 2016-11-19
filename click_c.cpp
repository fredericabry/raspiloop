#include "click_c.h"
#include "qdebug.h"
#include "playback_loop_c.h"
#include "playback_port_c.h"
#include "parameters.h"


click_c::click_c(int tempo, playback_port_c *pPort, int status):tempo(tempo),pPort(pPort),status(status)

{
    if(tempo<=0) {qDebug()<<"invalid tempo";delete this;return;}
    t1 = new QElapsedTimer;
    t1->start();

    long deltams = 60000/tempo; //time in ms between ticks

    connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));
    timer.start(deltams);





}





void click_c::tick(void)
{
    //qDebug()<<t1->elapsed();

    //qDebug()<<"\ntick";
    if(status == STATUS_PLAY)
        new playback_loop_c(0,pPort,0,true);//once, autoplay
    //t1->start();
}

