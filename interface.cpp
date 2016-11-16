#include "interface.h"
#include "capture_loop_c.h"
#include "alsa_capture.h"
#include "alsa_playback.h"
#include "qdebug.h"

#define MAX_PLAY_LOOPS 5
#define MAX_CAPTURE_LOOPS 5

playback_loop_c *pPlayLoopList[MAX_PLAY_LOOPS];
capture_loop_c *pCaptureLoopList[MAX_PLAY_LOOPS];

int playLoopNumber;



playback_loop_c  *pPlayLoop0;


capture_loop_c *pCaptureLoop1;
capture_loop_c *pCaptureLoop0;

playback_port_c *pLeft,*pRight;
capture_port_c *pRec0, *pRec1 ;


capture_port_c *pActiveRecPort;
playback_port_c *pActivePlayPort;
playback_loop_c *pActivePlay;




playback_loop_c* interface_c::findLastPlaybackLoop(void) //return a pointer to the last playloop
{
    playback_loop_c *pLoop = firstLoop;
    if(firstLoop == NULL)
    {
        return NULL;
    }

    while(pLoop->pNextLoop != NULL)
        pLoop = pLoop->pNextLoop;

    return pLoop;

}
int interface_c::getPlayLoopCount()
{
    int i = 0;
    playback_loop_c *pLoop = firstLoop;


    if(pLoop == NULL) return 0;
    else
    {
        while(pLoop->pNextLoop!=NULL) {pLoop=pLoop->pNextLoop;i++;}
    }

    return i;


}





interface_c::interface_c(MainWindow *parent):parent(parent)
{
firstLoop = NULL;//no loop at this point

}

void interface_c::run()
{

    alsa_start_playback("hw:1,0", 2,RATE,this);
    alsa_start_capture("hw:1,0", 2, RATE,this);



    QThread::sleep(0.5);

    pLeft = alsa_playback_port_by_num(0);
    pRight = alsa_playback_port_by_num(1);
    pRec0 = alsa_capture_port_by_num(0);
    pRec1 = alsa_capture_port_by_num(1);


    pActiveRecPort = NULL;




    exec();
}

void interface_c::keyInput(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_1:
        pActiveRecPort = pRec0;
        qDebug()<<"Input"<<pActiveRecPort->id<<"selected";

        break;
    case Qt::Key_2:
        if(pActiveRecPort == NULL)
        {
            qDebug()<<"use keys 1 and 3 to select the input";
            return;
        }
        //let's do some recording


        break;
    case Qt::Key_3:
        pActiveRecPort = pRec1;
        qDebug()<<"Input"<<pActiveRecPort->id<<"selected";

        //  pPlayLoop0= new playback_loop_c("loop 2_1","rec00.wav",pLeft,-1);
        //  pPlayLoop1= new playback_loop_c("loop 2_2","rec10.wav",pRight,-1);

        break;
    case Qt::Key_4:qDebug()<<4;
        pPlayLoop0= new playback_loop_c("loop 2_1","nantesg.wav",pLeft,5000);
        pPlayLoop0->play();
        break;
    case Qt::Key_5:qDebug()<<5;


        break;
    case Qt::Key_6:qDebug()<<6;break;
    case Qt::Key_7:qDebug()<<7;break;
    case Qt::Key_8:qDebug()<<8;break;
    case Qt::Key_9:qDebug()<<getPlayLoopCount(); break;
    case Qt::Key_Enter:

        parent->close();


        break;
    }


}

void interface_c::Afficher(QString txt)
{
    qDebug()<<txt;

}
