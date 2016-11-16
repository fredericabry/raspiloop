#include "playback_loop_c.h"
#include <qdebug.h>
#include "QElapsedTimer"
#include "parameters.h"
#include "interface.h"


playback_loop_c::playback_loop_c(int id,  playback_port_c *pPort, long length,bool autoplay):id(id),pPort(pPort)
{



    filename = QString::number(id)+".wav";

    pPrevLoop = pPort->interface->findLastPlaybackLoop();
    pNextLoop = NULL;//last loop created.

    if(autoplay) status = STATUS_PLAY;
    else
    status = STATUS_IDLE;//at first the loop is not playing.

    consumer = new playbackLoopConsumer;
    consumer->controler = this;
    consumer->loopActive = false;



    if( !connect(consumer,SIGNAL(finished()), this, SLOT(consumerKilled())))
        qDebug()<<"connection failed";


    buffile = (short*)malloc(sizeof(short)*NFILE_PLAYBACK);


    frametoplay = (length*RATE)/1000;



    repeat = false;
    if(frametoplay==0)
        stop = false;
    else if(frametoplay<0)
    {
        stop = false;
        repeat = true;

    }

    else
    {
        stop = true;



    }
    openFile();

    pPort->addloop(this);




    consumer->start();




}




void playback_loop_c::openFile()
{
    SF_INFO sf_info;


    QString filename2 = DIRECTORY+filename;

    if ((this->soundfile = sf_open (filename2.toStdString().c_str(), SFM_RDWR, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile \"%s\" for output (%s)\n",filename.toStdString().c_str(), errstr);


        destroyloop(false);
        return;
    }


    //   qDebug()<<"created 2 "<<id;


    int short_mask = SF_FORMAT_PCM_16;


    if(sf_info.format != (SF_FORMAT_WAV|short_mask))
    {
        qDebug()<<"format de fichier incorrect\n";
        destroyloop(true);
        return;
    }
    /*
    if(sf_info.samplerate != rate_play)
    {
        parent_play->Afficher("soundfile rate incorrect\n");
        return;
    }
    */
    if(sf_info.channels != 1)
    {
        qDebug()<<"chan nbr incorrect\n";
        destroyloop(true);
        return;
    }


}

void playback_loop_c::consumerKilled() //the consumer is closed, we can kill the loop thread properly
{


    delete this;//let's kill the loop
}

void playback_loop_c::play()
{

    this->status = STATUS_PLAY;

}


void playback_loop_c::pause()
{

    this->status = STATUS_IDLE;

}

void playback_loop_c::moveToPort(playback_port_c *pNuPort)
{

 if(!pNuPort) {qDebug()<<"invalid port";return;}
 pPort->removeloop(this);
 pPort = pNuPort;
 pPort->addloop(this);

}


void playback_loop_c::destroyloop(bool opened)
{


    if (opened)
    {
        sf_close(soundfile);

        this->pPort->removeloop(this);

    }

    consumer->quit();


}

playback_loop_c::~playback_loop_c(void)
{
    free(buffile);

 //qDebug()<<"playback loop destroyed";
}





void playbackLoopConsumer::run()
{

    exec();


}

QElapsedTimer t3;

void playbackLoopConsumer::datarequest(int frames)
{


    int nread,t1;
    static int tmax = 0;

    //first request only activate the loop to avoid synchronization issues
    if(!loopActive) {
        //qDebug()<<"not yet";
        //qDebug()<<this->controler->pPort->connected_loops;
        //qDebug()<<this->controler->pPort->nu_connected_loops;

                     return;}

   // qDebug()<<"received"<<this->controler->pPort->connected_loops;


  //  qDebug()<<"received"<<controler->pPort->connected_loops;

    //the associated playback port requests more data

    if(controler->status == STATUS_IDLE)
    {
        //pausing, not sending any data
        emit send_data(controler->buffile,0);//we still need to answer to the data request or the playback port get stuck waiting for data

        return;

    }

    if(frames>NFILE_PLAYBACK) frames = NFILE_PLAYBACK;

    t3.start();

    sf_command (controler->soundfile, SFC_UPDATE_HEADER_NOW, NULL, 0) ; //update the file length, we might be writing on it in some other thread



    if((nread = sf_readf_short(controler->soundfile,controler->buffile,frames))>0)
    {

        if(controler->stop)
        {
            controler->frametoplay -= nread;
        }
        emit send_data(controler->buffile,nread);
    }


    t1 = t3.elapsed();
    if(t1 > tmax) tmax = t1;
    if (t1 > 50) qDebug()<<"load delay: "<<t1<<"ms max : "<<tmax<<" ms"<<nread;

    if((controler->stop&&(controler->frametoplay<=0))||(nread <= 0))
    {

        if(controler->repeat)
        {
            sf_seek(controler->soundfile,0,SFM_READ);
        }
        else
        {

            controler->destroyloop(true);
            //qDebug()<<"stop";

        }

        emit send_data(controler->buffile,0);//we still need to answer to the data request or the playback port get stuck waiting for data



  //    qDebug()<<"fin"<<nread;

    }




}


void playbackLoopConsumer::activate()
{
    loopActive = true;
}
