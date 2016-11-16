#include "interface.h"
#include "capture_loop_c.h"


#define MAX_PLAY_LOOPS 5
#define MAX_CAPTURE_LOOPS 5

playback_loop_c *pPlayLoopList[MAX_PLAY_LOOPS];
capture_loop_c *pCaptureLoopList[MAX_PLAY_LOOPS];

int playLoopNumber;






capture_loop_c *pCaptureLoop1;
capture_loop_c *pCaptureLoop0;

playback_port_c *pLeft,*pRight;
capture_port_c *pRec0, *pRec1 ;


capture_port_c *pActiveRecPort;
playback_port_c *pActivePlayPort;
playback_loop_c *pActivePlay;




playback_loop_c* findLastPlaybackLoop(playback_loop_c *pStartPoint) //return a pointer to the last playloop
{

    while(pStartPoint->pNextLoop != NULL)
        pStartPoint = pStartPoint->pNextLoop;

    return pStartPoint;

}


int loopNumber(playback_loop_c *pStartPoint)
{
    int i = 0;
    playback_loop_c *p = pStartPoint;


    if(pStartPoint == NULL) return 0;
    else
    {
        while(p->pPrevLoop!=NULL) {p=p->pPrevLoop;i++;}
        p = pStartPoint;
        while(p->pNextLoop!=NULL) {p=p->pNextLoop;i++;}
    }

    return i;


}


playback_loop_c* createNewPlayLoop()
{
playback_loop_c* Last = findLastPlaybackLoop(pActivePlay);

if(Last == NULL)
{
    qDebug()<<"no loop so far";
    pActivePlay = new playback_loop_c("1","play1.wav",pActivePlayPort,-1,NULL);//first loop
}

int n = loopNumber(pActivePlay);


}



interface_c::interface_c(MainWindow *parent):parent(parent)
{


}

void interface_c::run()
{

    alsa_start_playback("hw:1,0", 2,RATE);
    alsa_start_capture("hw:1,0", 2, RATE);



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

        break;
    case Qt::Key_5:qDebug()<<5;


        break;
    case Qt::Key_6:qDebug()<<6;break;
    case Qt::Key_7:qDebug()<<7;break;
    case Qt::Key_8:qDebug()<<8;break;
    case Qt::Key_9:qDebug()<<9;break;
    case Qt::Key_Enter:

        parent->close();


        break;
    }


}

