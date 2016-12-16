#include "click_c.h"
#include "qdebug.h"
#include "playback_loop_c.h"
#include "playback_port_c.h"
#include "parameters.h"
#include <sndfile.h>



click_c::click_c(int nuTempo, std::vector<playback_port_c*> pPorts, status_t status, interface_c *interface, MainWindow *parent):pPorts(pPorts),status(status),interface(interface),parent(parent)
{
    if(nuTempo<=0) {qDebug()<<"invalid tempo";delete this;return;}
    t1 = new QElapsedTimer;
    setTempo(nuTempo);
    beat = 1;



    preload();


    long deltams = 60000/nuTempo; //time in ms between ticks

    QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(tick()));
    timer.start(deltams);

    //connect(parent->ClickUp,SIGNAL(pressed()),this,SLOT(setTempo(int)));
    connect(parent,SIGNAL(clickUp()),this,SLOT(clickUp()));
    connect(parent,SIGNAL(clickDown()),this,SLOT(clickDown()));




    //  new playback_loop_c(1,pPorts,0,CLICKSYNC,PLAY,interface,0);//once, autoplay
}



void click_c::preload()
{

    SF_INFO sf_info;
    SNDFILE *soundfile;
    QString filename = (QString)DIRECTORY+"click0.wav";
    if ((soundfile = sf_open (filename.toStdString().c_str(), SFM_READ, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        qDebug()<< "cannot open sndfile"<< filename.toStdString().c_str()<<"for output"<<errstr;
        return;
    }
    int short_mask = SF_FORMAT_PCM_16;
    if(sf_info.format != (SF_FORMAT_WAV|short_mask))
    {
       qDebug()<<"format de fichier incorrect\n";
       return;
    }

    if(sf_info.channels != 1)
    {
        qDebug()<<"chan nbr incorrect\n";
        return;
    }

    long nrequest = sf_info.frames;
    bufClick0 = new short[nrequest];
    bufClick0_L = nrequest;
    sf_readf_short(soundfile,bufClick0,nrequest);
    sf_close(soundfile);


    filename = (QString)DIRECTORY+"click1.wav";
    if ((soundfile = sf_open (filename.toStdString().c_str(), SFM_READ, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        qDebug()<< "cannot open sndfile"<< filename.toStdString().c_str()<<"for output"<<errstr;
        return;
    }

    if(sf_info.format != (SF_FORMAT_WAV|short_mask))
    {
       qDebug()<<"format de fichier incorrect\n";
       return;
    }

    if(sf_info.channels != 1)
    {
        qDebug()<<"chan nbr incorrect\n";
        return;
    }

    nrequest = sf_info.frames;
    bufClick1 = new short[nrequest];
    bufClick1_L = nrequest;
    sf_readf_short(soundfile,bufClick1,nrequest);
    sf_close(soundfile);



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
    t1->start();



    beat ++;
    if(beat>4) beat = 1;
    if(beat == 1)
    {if(status == PLAY)
        {
            for(auto pPort : pPorts)
            {
                pPort->clickDataToPlay0 =  bufClick0_L;
            }
        }

        emit firstBeat();


    }
    else
    {

        if(status == PLAY)
        {
            for(auto pPort : pPorts)
            {
                pPort->clickDataToPlay1 =  bufClick1_L;
            }
        }
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

    //qDebug()<<"new tempo:"<<tempo;

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
        parent->setClickButton(false);

    }
    else
    {
        status = PLAY;
        parent->setClickButton(true);
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

