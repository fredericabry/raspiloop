#include "interface.h"
#include "capture_loop_c.h"
#include "playback_loop_c.h"
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
capture_loop_c *pActiveRecLoop;

playback_port_c *pActivePlayPort;
playback_loop_c *pActivePlayLoop;





playback_loop_c* interface_c::findLastPlaybackLoop(void) //return a pointer to the last playloop
{
    playback_loop_c *pLoop = firstPlayLoop;
    if(firstPlayLoop == NULL)
    {
        return NULL;
    }

    while(pLoop->pNextLoop != NULL)
        pLoop = pLoop->pNextLoop;

    return pLoop;

}
int interface_c::getPlayLoopsCount()
{
    int i = 0;
    playback_loop_c *pLoop = firstPlayLoop;


    if(pLoop == NULL) return 0;
    else
    {
        i=1;
        while(pLoop->pNextLoop!=NULL) {pLoop=pLoop->pNextLoop;i++;}
    }

    return i;


}

capture_loop_c* interface_c::findLastCaptureLoop(void) //return a pointer to the last playloop
{
    capture_loop_c *pLoop = firstCaptureLoop;
    if(firstCaptureLoop == NULL)
    {
        return NULL;
    }

    while(pLoop->pNextLoop != NULL)
        pLoop = pLoop->pNextLoop;

    return pLoop;

}

int interface_c::getCaptureLoopsCount()
{
    int i = 0;
    capture_loop_c *pLoop = firstCaptureLoop;


    if(pLoop == NULL) return 0;
    else
    {
        i=1;
        while(pLoop->pNextLoop!=NULL) {pLoop=pLoop->pNextLoop;i++;}
    }

    return i;


}

interface_c::interface_c(MainWindow *parent):parent(parent)
{
    firstPlayLoop = NULL;//no playback loop at this point
    firstCaptureLoop = NULL;//no capture loop at this point

}

int interface_c::generateNewId()
{
    int id = 0;
    playback_loop_c *pPlayLoop = firstPlayLoop;
    capture_loop_c *pCaptureLoop = firstCaptureLoop;
    if(pPlayLoop != NULL)
    {
        if(pPlayLoop->id >= id) id = pPlayLoop->id+1;
        while(pPlayLoop->pNextLoop!=NULL)
        {
            pPlayLoop=pPlayLoop->pNextLoop;
            if(pPlayLoop->id >= id) id = pPlayLoop->id+1;
        }
    }

    if(pCaptureLoop != NULL)
    {
        if(pCaptureLoop->id >= id) id = pCaptureLoop->id+1;
        while(pCaptureLoop->pNextLoop!=NULL)
        {
            pCaptureLoop=pCaptureLoop->pNextLoop;
            if(pCaptureLoop->id >= id) id = pCaptureLoop->id+1;
        }
    }


    return id;

}

void interface_c::removeAllPlaybackLoops(void)
{
    while(firstPlayLoop != NULL)
    {
        firstPlayLoop->destroyloop(true);

    }
    pActivePlayLoop = NULL;
}


void interface_c::init(void)
{

    alsa_start_playback("hw:1,0", 2,RATE,this);
    alsa_start_capture("hw:1,0", 2, RATE,this);
    //QThread::sleep(0.5);
    pLeft = alsa_playback_port_by_num(0);
    pRight = alsa_playback_port_by_num(1);
    pRec0 = alsa_capture_port_by_num(0);
    pRec1 = alsa_capture_port_by_num(1);
    pActiveRecPort = NULL;
    isRecording = false;


}

void interface_c::run()
{
    init();
    exec();
}







void interface_c::keyInput(QKeyEvent *e)
{
    // qDebug()<<e->key();
    switch(e->key())
    {


    case Qt::Key_Period:
        pActivePlayPort = pLeft;
        qDebug()<<"Output"<<pActivePlayPort->channel<<"selected";

        break;

    case Qt::Key_0:
        pActivePlayPort = pRight;
        qDebug()<<"Output"<<pActivePlayPort->channel<<"selected";
        break;

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
        if(pActivePlayPort == NULL)
        {
            qDebug()<<"use keys 0 and 1 to select the output";
            return;
        }
        //let's do some recording
        if(!isRecording)
        {
            qDebug()<<"recording...";
            isRecording = true;
            int id = generateNewId();//get some not used id
            pActiveRecLoop = new capture_loop_c(id,pActiveRecPort,0,true,pActivePlayPort);

        }
        else
        {
            qDebug()<<"recording stopped";
            isRecording = false;
            pActiveRecLoop->destroyLoop();

        }

        break;
    case Qt::Key_3:
        pActiveRecPort = pRec1;
        qDebug()<<"Input"<<pActiveRecPort->id<<"selected";
        break;
    case Qt::Key_4:
        if(!pActivePlayLoop) pActivePlayLoop = this->firstPlayLoop;

        if(pActivePlayLoop)
        {
            if(pActivePlayLoop->pPrevLoop) pActivePlayLoop = pActivePlayLoop->pPrevLoop;
            else pActivePlayLoop = findLastPlaybackLoop();
        }

        if(pActivePlayLoop) qDebug()<<"play loop #"<<pActivePlayLoop->id<<"selected";
        else qDebug()<<"no play loop available";


        break;
    case Qt::Key_5:
        if(pActivePlayLoop)
        {
            if(pActivePlayLoop->status == STATUS_IDLE) {pActivePlayLoop->play();qDebug()<<"loop"<<pActivePlayLoop->id<<"play";}
            else if(pActivePlayLoop->status == STATUS_PLAY) {pActivePlayLoop->pause();qDebug()<<"loop"<<pActivePlayLoop->id<<"paused";}

        }
        else
            qDebug()<<"no playloop active";


        break;
    case Qt::Key_6:

        if(!pActivePlayLoop) pActivePlayLoop = this->firstPlayLoop;

        if(pActivePlayLoop)
        {
            if(pActivePlayLoop->pNextLoop) pActivePlayLoop = pActivePlayLoop->pNextLoop;
            else pActivePlayLoop = firstPlayLoop;
        }

        if(pActivePlayLoop) qDebug()<<"play loop #"<<pActivePlayLoop->id<<"selected";
        else qDebug()<<"no play loop available";



        break;
    case Qt::Key_7:
        if(pActivePlayLoop)
        {
            if(pActivePlayLoop->pPort->channel == 1) {pActivePlayLoop->moveToPort(pLeft);}
            else pActivePlayLoop->moveToPort(pRight);

        }
        else
            qDebug()<<"no playloop active";

        break;
    case Qt::Key_8:qDebug()<<"Active capture loops count:"<<getCaptureLoopsCount();break;
    case Qt::Key_9:qDebug()<<"Active playback loops count:"<<getPlayLoopsCount(); break;
    case Qt::Key_Enter:

        parent->close();


        break;


    case Qt::Key_Minus:this->removeAllPlaybackLoops();qDebug()<<"All playback loops deleted"; break;
    }


}

void interface_c::Afficher(QString txt)
{
    qDebug()<<txt;

}