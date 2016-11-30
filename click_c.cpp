#include "click_c.h"
#include "qdebug.h"
#include "playback_loop_c.h"
#include "playback_port_c.h"
#include "parameters.h"




click_c::click_c(int tempo, playback_port_c *pPort, status_t status, interface_c *interface, MainWindow *parent):pPort(pPort),status(status),interface(interface),parent(parent)
{
    if(tempo<=0) {qDebug()<<"invalid tempo";delete this;return;}
    t1 = new QElapsedTimer;




    long deltams = 60000/tempo; //time in ms between ticks

    QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));
    timer.start(deltams);

    //connect(parent->ClickUp,SIGNAL(pressed()),this,SLOT(setTempo(int)));
    connect(parent,SIGNAL(clickUp()),this,SLOT(clickUp()));
    connect(parent,SIGNAL(clickDown()),this,SLOT(clickDown()));


    setTempo(tempo);
    beat = 1;


}

double click_c::getBeat(void)
{
    double t = ((double)t1->elapsed())/1000;//time elapsed since the tick in seconds

    if(t<0) {qDebug()<<"bug elapsed beat"<<beat; return beat;}


    return (double)beat-1+t*tempo/60; //value in [0,4[
}

double click_c::getTime(void)
{
    double t = ((double)t1->elapsed())/1000;//time elapsed since the tick in seconds

    return t +(double)(beat-1)*60/tempo;


}

void click_c::clickDown()
{
    setTempo(getTempo()-5);

}


void click_c::clickUp()
{

    setTempo(getTempo()+5);
}


void click_c::tick(void)
{
    playback_port_c **pPorts = new playback_port_c*[1];
    pPorts[0] = pPort;

    t1->start();
    beat ++;
    if(beat>4) beat = 1;
    if(beat == 1)
    {
        emit firstBeat();

        if(status == PLAY)
            new playback_loop_c(0,pPorts,1,0,NOSYNC,PLAY,interface);//once, autoplay

        // qDebug()<<"first beat";
    }
    else
    {
        if(status == PLAY)
            new playback_loop_c(1,pPorts,1,0,NOSYNC,PLAY,interface);//once, autoplay
    }


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

    parent->setClickText(temp);


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



void click_c::stop(void)
{
    status = IDLE;
    beat = 1;


}

void click_c::start(void)
{


    status = PLAY;

}

