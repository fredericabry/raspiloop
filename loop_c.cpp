#include "loop_c.h"
#include <qdebug.h>


loop_c::loop_c()
{

}


void loop_c::destroyloop()
{

        delete this;
}



void loop_c::init(QString filename,ringbuf_c *pRingBuffer)
{

    SF_INFO sf_info;

    buffile = (short*)malloc(sizeof(short)*bufilesize);

    pRing = pRingBuffer;


    connect(pRing,SIGNAL(signal_trigger()), this, SLOT(datarequest()));


    if ((this->soundfile = sf_open (filename.toStdString().c_str(), SFM_READ, &sf_info)) == NULL) {
        char errstr[256];
        sf_error_str (0, errstr, sizeof (errstr) - 1);
        fprintf (stderr, "cannot open sndfile for output %s\n", errstr);

      destroyloop();
      return;
    }

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









}


void loop_c::datarequest(void)
{
    //the associated ringbuffer request more data


    int nread;



//qDebug()<<"data request";

    if((nread = sf_readf_short(soundfile,buffile,NFILE))>0)
    {


        pRing->pushN(buffile,nread);

    }

    if(nread == 0)
    {
        sf_close(soundfile);
        this->soundfile = NULL;
        this->destroyloop();


    }

}



loop_c::~loop_c(void)
{

    free(buffile);

}
