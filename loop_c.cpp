#include "loop_c.h"
#include <qdebug.h>
#include "mainwindow.h"






loop_c::loop_c(const QString id, const QString filename, playback_port_c *pRing2, int length):id(id),filename(filename)
{

    pRing = pRing2;
        SF_INFO sf_info;

    if(!connect(pRing,SIGNAL(signal_trigger(int)), this, SLOT(datarequest(int))))
        qDebug()<<"connection failed";

   if( !connect(this,SIGNAL(send_data(short*,int)), pRing, SLOT(data_available(short*, int))))
                   qDebug()<<"connection failed";

    if ((this->soundfile = sf_open (filename.toStdString().c_str(), SFM_READ, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile for output %s\n", errstr);

             //debugf("loop failed ; cannot open sndfile for output ");

        destroyloop();
        return;
    }







    buffile = (short*)malloc(sizeof(short)*NFILE_PLAYBACK);
    frametoplay = length;
    repeat = false;
    if(frametoplay==0)
        stop = false;
    else if(frametoplay<0)
    {
        stop = false;
        repeat = true;

    }

    else stop = true;




    int short_mask = SF_FORMAT_PCM_16;

    if(sf_info.format != (SF_FORMAT_WAV|short_mask))
    {
        qDebug()<<"format de fichier incorrect\n";
        destroyloop();
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
        destroyloop();
        return;
    }





   pRing->addloop(this);



}


void loop_c::destroyloop()
{
  //  debugf("loop "+id+" destroyed");
    this->pRing->removeloop();
    delete this;
}


void loop_c::test(QString c)
{
    qDebug()<<c;

}




void loop_c::datarequest(int frames)
{
    //the associated ringbuffer request more data



    int nread;


    if(frames>NFILE_PLAYBACK) frames = NFILE_PLAYBACK;

//debugf(id+" data request received");



    if((nread = sf_readf_short(soundfile,buffile,frames))>0)
    {

        if(stop)
        {
            this->frametoplay -= nread;

        }
        //pRing->data_available(buffile,nread);
        emit send_data(buffile,nread);
            //debugf(id+" loop sends data :" +n2s(nread) + " samples");


    }

    if(            (stop&&(frametoplay<=0))
                   ||(nread == 0))
    {
        if(repeat)
        {
            sf_seek(soundfile,0,SFM_READ);

        }

        else{
            sf_close(soundfile);
            this->soundfile = NULL;
            this->destroyloop();

        }

        emit send_data(buffile,0);//we still need to answer to the data request or the playback port get stuck waiting for data
        //pRing->data_available(buffile,0);
            //debugf("loop sends data :  0 sample");
    }




}

loop_c::~loop_c(void)
{
    free(buffile);

}
