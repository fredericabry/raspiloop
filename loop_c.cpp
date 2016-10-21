#include "loop_c.h"
#include <qdebug.h>
#include "mainwindow.h"






loop_c::loop_c(const QString id, const QString filename, playback_port_c *pRing2, int length):id(id),filename(filename)
{


   //     qDebug()<<"created 0 "<<id;

    pRing = pRing2;
    SF_INFO sf_info;

    if(!connect(pRing,SIGNAL(signal_trigger(int)), this, SLOT(datarequest(int))))
        qDebug()<<"connection failed";

    if( !connect(this,SIGNAL(send_data(short*,int)), pRing, SLOT(data_available(short*, int))))
        qDebug()<<"connection failed";

     buffile = (short*)malloc(sizeof(short)*NFILE_PLAYBACK);

 //   qDebug()<<"created 1 "<<id;



    if ((this->soundfile = sf_open (filename.toStdString().c_str(), SFM_READ, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile \"%s\" for output (%s)\n",filename.toStdString().c_str(), errstr);


            destroyloop(false);
            return;
         }


  //   qDebug()<<"created 2 "<<id;





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


 //   qDebug()<<"created 3 "<<id;


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




 //    qDebug()<<"created 4 "<<id;

    pRing->addloop(this);


 //  qDebug()<<"created "<<id;

   //   qDebug()<<"loop created "<<id;

}


void loop_c::destroyloop(bool opened)
{

    if(!disconnect(pRing,SIGNAL(signal_trigger(int)), this, SLOT(datarequest(int))))
        qDebug()<<"disconnection failed";

    if( !disconnect(this,SIGNAL(send_data(short*,int)), pRing, SLOT(data_available(short*, int))))
        qDebug()<<"disconnection failed";


if (opened)
{
    sf_close(soundfile);

    this->pRing->removeloop();
}




 //    qDebug()<<"loop destroyed "<<id;

    delete this;
}






void loop_c::datarequest(int frames)
{
    //the associated ringbuffer request more data

    // qDebug()<<"data request "<<id;

    int nread;


    if(frames>NFILE_PLAYBACK) frames = NFILE_PLAYBACK;

    //qDebug()<<id<<" data request received";

    sf_read_short(soundfile,buffile,0) ;

    if((nread = sf_readf_short(soundfile,buffile,frames))>0)
    {
        if(stop)
        {
            this->frametoplay -= nread;
        }
        emit send_data(buffile,nread);
    }

    if((stop&&(frametoplay<=0))||(nread <= 0))
    {
        if(repeat)
        {
            sf_seek(soundfile,0,SFM_READ);
        }

        else
        {
            this->destroyloop(true);
        }

        emit send_data(buffile,0);//we still need to answer to the data request or the playback port get stuck waiting for data
        //pRing->data_available(buffile,0);

    }




}

loop_c::~loop_c(void)
{
    free(buffile);

}
