#include "playback_loop_c.h"
#include <qdebug.h>
#include "mainwindow.h"

#include "QElapsedTimer"




playback_loop_c::playback_loop_c(const QString id, const QString filename, playback_port_c *pRing, int length):id(id),filename(filename),pRing(pRing)
{


    //  qDebug()<<"Loop created "<<id;



    consumer = new playbackLoopConsumer;
    consumer->controler = this;
        if(!connect(pRing,SIGNAL(signal_trigger(int)), consumer, SLOT(datarequest(int))))
        qDebug()<<"connection failed";

    if( !connect(consumer,SIGNAL(send_data(short*,int)), pRing->consumer, SLOT(data_available(short*, int))))
        qDebug()<<"connection failed";




    buffile = (short*)malloc(sizeof(short)*NFILE_PLAYBACK);


    frametoplay = (length*441)/10;
    repeat = false;
    if(frametoplay==0)
        stop = false;
    else if(frametoplay<0)
    {
        stop = false;
        repeat = true;

    }

    else stop = true;

    openFile();

    pRing->addloop(this);




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






void playback_loop_c::destroyloop(bool opened)
{

    if(!disconnect(pRing,SIGNAL(signal_trigger(int)), consumer, SLOT(datarequest(int))))
        qDebug()<<"playback loop disconnection failed 1";

    if( !disconnect(consumer,SIGNAL(send_data(short*,int)), pRing->consumer, SLOT(data_available(short*, int))))
        qDebug()<<"playback loop  disconnection failed 2";



    consumer->quit();


    if (opened)
    {
        sf_close(soundfile);

        this->pRing->removeloop();
    }





    delete this;
}

playback_loop_c::~playback_loop_c(void)
{
    free(buffile);
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
    //the associated playback port requests more data

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
        }

        emit send_data(controler->buffile,0);//we still need to answer to the data request or the playback port get stuck waiting for dataaaq



      qDebug()<<"fin"<<nread;

    }




}



