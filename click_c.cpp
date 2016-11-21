#include "click_c.h"
#include "qdebug.h"
#include "playback_loop_c.h"
#include "playback_port_c.h"
#include "parameters.h"


click_c::click_c(int tempo, playback_port_c *pPort, status_t status):tempo(tempo),pPort(pPort),status(status)

{
    if(tempo<=0) {qDebug()<<"invalid tempo";delete this;return;}
    t1 = new QElapsedTimer;
    t1->start();

    long deltams = 60000/tempo; //time in ms between ticks

    QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));
    timer.start(deltams);

    beat = 1;





}





void click_c::tick(void)
{


    if(beat == 1)
    {
        emit firstBeat();


        if(status == PLAY)
        new playback_loop_c(0,pPort,0,NOSYNC,PLAY);//once, autoplay


    }
    else
    {
        if(status == PLAY)
        new playback_loop_c(1,pPort,0,NOSYNC,PLAY);//once, autoplay

    }
    //qDebug()<<t1->elapsed();

    //qDebug()<<"\ntick";

    //t1->start();
    beat ++;

    if(beat>4) beat = 1;

}




bool click_c::isActive(void)
{

    if(status == PLAY) return true;
    else return false;

}


void click_c::setTempo(int temp)
{
    if(temp < 20) return;
    if(temp > 200) return;

    tempo = temp;
    long deltams = 60000/tempo; //time in ms between ticks
    timer.setInterval(deltams);

    qDebug()<<"new tempo:"<<tempo;

}

int click_c::getTempo()
{
    return tempo;
}

void click_c::stopstart(void)
{

    if(status == PLAY)
    {
        status = IDLE;
        beat = 1;

    }
    else
    {
        status = PLAY;

    }

}
