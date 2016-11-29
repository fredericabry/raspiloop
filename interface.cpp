
#include "capture_loop_c.h"
#include "playback_loop_c.h"
#include "alsa_capture.h"
#include "alsa_playback.h"
#include "interface.h"
#include "qdebug.h"
#include "click_c.h"

#include "events.h"


#define KEY_WAIT 200 //ms



playback_port_c *pLeft,*pRight;
capture_port_c *pRec0, *pRec1 ;

capture_port_c *pActiveRecPort;
capture_loop_c *pActiveRecLoop;

playback_port_c *pActivePlayPort;
playback_loop_c *pActivePlayLoop;

interfaceEvent_c *pActiveEvent;



playback_loop_c* interface_c::findLastPlaybackLoop(void) //return a pointer to the last playloop
{



    playback_loop_c *pLoop = firstPlayLoop;
    if(firstPlayLoop == NULL)
    {

        return NULL;

    }

    while(pLoop->pNextLoop != NULL)
    {
        pLoop = pLoop->pNextLoop;

    }


    return pLoop;

}

int interface_c::getPlayLoopsCount()
{
    int i = 0;
    playbackListMutex.lock();
    playback_loop_c *pLoop = firstPlayLoop;


    if(pLoop == NULL) {playbackListMutex.unlock();return 0;}
    else
    {
        i=1;
        while(pLoop->pNextLoop!=NULL) {pLoop=pLoop->pNextLoop;i++;}
    }
    playbackListMutex.unlock();
    return i;


}



capture_loop_c* interface_c::findLastCaptureLoop(void) //return a pointer to the last playloop
{
    //no mutex here, used inside a mutex


    capture_loop_c *pLoop = firstCaptureLoop;

    if(firstCaptureLoop == NULL)
    {
        return NULL;
    }

    if(pLoop == NULL) qDebug()<<"BUG FINDLAST CAPTURE";
    while(pLoop->pNextLoop != NULL)
    {


        if(!pLoop) qDebug()<<"BUG FINDLAST CAPTURE";
        pLoop = pLoop->pNextLoop;

    }


    return pLoop;

}

int interface_c::getCaptureLoopsCount()
{
    int i = 0;
    captureListMutex.lock();
    capture_loop_c *pLoop = firstCaptureLoop;


    if(pLoop == NULL) {captureListMutex.unlock();return 0;}
    else
    {
        i=1;
        while(pLoop->pNextLoop!=NULL) {pLoop=pLoop->pNextLoop;i++;}
    }

    captureListMutex.unlock();
    return i;


}


interface_c::interface_c(MainWindow *parent):parent(parent)
{
    firstPlayLoop = NULL;//no playback loop at this point
    firstCaptureLoop = NULL;//no capture loop at this point
    firstEvent = NULL;//no event at this point

}

int interface_c::generateNewId()
{
    int id = 2; //id = 0 and id =1 are reserved for the click


    playbackListMutex.lock();
    playback_loop_c *pPlayLoop = firstPlayLoop;
    if(pPlayLoop != NULL)
    {
        if(pPlayLoop->id >= id) id = pPlayLoop->id+1;
        while(pPlayLoop->pNextLoop!=NULL)
        {
            pPlayLoop=pPlayLoop->pNextLoop;
            if(pPlayLoop->id >= id) id = pPlayLoop->id+1;
        }
    }
    playbackListMutex.unlock();
    captureListMutex.lock();
    capture_loop_c *pCaptureLoop = firstCaptureLoop;
    if(pCaptureLoop != NULL)
    {
        if(pCaptureLoop->id >= id) id = pCaptureLoop->id+1;
        while(pCaptureLoop->pNextLoop!=NULL)
        {
            pCaptureLoop=pCaptureLoop->pNextLoop;
            if(pCaptureLoop->id >= id) id = pCaptureLoop->id+1;
        }
    }
    captureListMutex.unlock();

    return id;

}

playback_loop_c* interface_c::findPlayLoopById(int id)
{
    playbackListMutex.lock();
    playback_loop_c *pLoop = firstPlayLoop;
    while(pLoop != NULL)
    {
        if(pLoop->id == id) {playbackListMutex.unlock();return pLoop;}
        pLoop = pLoop->pNextLoop;
    }
    playbackListMutex.unlock();
    return NULL;
}


int interface_c::getEventsCount()
{
    eventListMutex.lock();

    int i = 0;
    interfaceEvent_c *pEvent = firstEvent;


    if(pEvent == NULL) {eventListMutex.unlock();return 0;}
    else
    {
        i=1;
        while(pEvent->pNextEvent!=NULL) {pEvent=pEvent->pNextEvent;i++;}
    }

    eventListMutex.unlock();
    return i;


}

void interface_c::showPlayLoops()
{
    playback_loop_c *pLoop = firstPlayLoop;
    while(pLoop != NULL)
    {
        qDebug()<<"loop" <<pLoop->id;
        if(pLoop->pPrevLoop)qDebug()<<"prev"<<pLoop->pPrevLoop->id;
        if(pLoop->pNextLoop)qDebug()<<"next"<<pLoop->pNextLoop->id;

        qDebug()<<"\n";
        pLoop = pLoop->pNextLoop;

    }


}

void interface_c::removeAllPlaybackLoops(void)
{

    playback_loop_c *pLoop = firstPlayLoop;
    playback_loop_c *pLoop2 = NULL;
    while(pLoop != NULL)
    {

        pLoop2 = pLoop->pNextLoop;

        pLoop->destroy();

        pLoop = pLoop2;

    }
    pActivePlayLoop = NULL;

}

interfaceEvent_c* interface_c::findLastEvent(void) //return a pointer to the last event in the list
{

    interfaceEvent_c* pEvent = firstEvent;

    if(pEvent == NULL)
    {

        return NULL;
    }

    while(pEvent->pNextEvent)
    {
        pEvent = pEvent->pNextEvent;
    }

    return pEvent;

}

bool interface_c::isEventValid(interfaceEvent_c* pEvent)
{
    eventListMutex.lock();
    interfaceEvent_c* pEvent2 = firstEvent;

    if(pEvent2 == NULL)
    {
        eventListMutex.unlock();
        return false;
    }

    if(pEvent == NULL)
    {
        eventListMutex.unlock();
        return false;
    }

    if(pEvent2==pEvent)
    {
        eventListMutex.unlock();
        return true;
    }


    while(pEvent2->pNextEvent != NULL)
    {
        pEvent2 = pEvent2->pNextEvent;
        if(pEvent2==pEvent) {eventListMutex.unlock(); return true;}
    }

    eventListMutex.unlock();
    return false;
}


void interface_c::clickPlayStop(void)
{
    if(clickStatus) {
        clickStatus = false;
        pClick->stop();

    }
    else
    {
        clickStatus = true;
        //play
        pClick->start();
    }

    parent->setClickButton(clickStatus);

}

QString interface_c::statusToString(status_t status){
    switch(status)
    {
    case IDLE:return "stopped";break;
    case PLAY:return "playing";break;
    case SILENT:return "silent";break;
    default:return "unknown";break;

    }


}

void interface_c::printLoopList(void)
{
    playback_loop_c *pLoop = this->firstPlayLoop;
    QString txt = "";


    if(getPlayLoopsCount() <= 0) txt="no loop";
    else
    {

        if(pLoop)
        {
            txt+="Loop #"+QString::number(pLoop->id)+":"+statusToString(pLoop->status)+"\n";

        }

        while(pLoop->pNextLoop)
        {
            pLoop = pLoop->pNextLoop;
           if(pLoop)  txt+="Loop #"+QString::number(pLoop->id)+":"+statusToString(pLoop->status)+"\n";

        }
    }

    emit loopList(txt);

}




bool interface_c::isCaptureLoopValid(capture_loop_c * pCapture)
{

    capture_loop_c* pCapture2 = firstCaptureLoop;

    if(pCapture2 == NULL)
    {

        return false;
    }

    if(pCapture == NULL)
    {

        return false;
    }

    if(pCapture2==pCapture)
    {

        return true;
    }


    while(pCapture2->pNextLoop != NULL)
    {
        pCapture2 = pCapture2->pNextLoop;
        if(pCapture2==pCapture) {return true;}
    }


    return false;
}


void interface_c::removeAllEvents(void)
{


    interfaceEvent_c *pEvent = firstEvent;
    interfaceEvent_c *pEvent2 = NULL;
    while(pEvent != NULL)
    {

        pEvent2 = pEvent->pNextEvent;
        pEvent->destroy();
        pEvent = pEvent2;
    }


}


void interface_c::createInterfaceEvent(const QObject * sender,const char * signal, int eventType, void *param,bool repeat,interfaceEvent_c **pEvent)
{

    if(pEvent)
        *pEvent = new interfaceEvent_c(sender,signal,eventType,param,this,repeat);
    else new interfaceEvent_c(sender,signal,eventType,param,this,repeat);




}


int interface_c::getCaptureStatus(capture_loop_c **pCaptureLoop)
{




    if(isCaptureLoopValid(*pCaptureLoop)) return 1;

    interfaceEvent_c *pEvent = firstEvent;


    if(pEvent == NULL) {qDebug()<<"no event";return 0;}
    else
    {
        if(pEvent->eventType==EVENT_CAPTURE)
        {

            if(pEvent->captureData->pCaptureLoop == pCaptureLoop) return 2;
        }
        while(pEvent->pNextEvent!=NULL) {
            pEvent=pEvent->pNextEvent;
            if(pEvent->eventType==EVENT_CAPTURE)
            {
                if(pEvent->captureData->pCaptureLoop == pCaptureLoop) return 2;
            }
        }
    }

    return 0;
}

void interface_c::startRecord(playback_port_c *pPlayPort,capture_port_c *pCapturePort,capture_loop_c **pCaptureLoop,long length)
{
    *pCaptureLoop = NULL;


    if(synchroMode == CLICKSYNC)
    {


        if(pClick->getBeat()<1) //capture was launched too late, we are not going to wait till the next bar...
        {
            *pCaptureLoop = new capture_loop_c(generateNewId(),pCapturePort,length,true,pPlayPort,pClick->getTime());

        }

        else
        {

            captureData_s *params = new captureData_s;
            params->createPlayLoop = true;
            params->length=length;
            params->pPlayPort=pPlayPort;
            params->pPort=pCapturePort;
            params->pCaptureLoop = pCaptureLoop;

            createInterfaceEvent(pClick,SIGNAL(firstBeat()),EVENT_CAPTURE,(void*)params,false,&pActiveEvent);

        }
    }

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
    synchroMode = CLICKSYNC;

    isRecording = false;




    pClick = new click_c(120,pLeft,IDLE,parent);
    clickStatus=false;
    connect(this,SIGNAL(setTempo(int)),pClick,SLOT(setTempo(int)));
    connect(this,SIGNAL(loopList(QString)),parent,SLOT(setLoopList(QString)));


    telapsed.start();
    //connect(pClick,SIGNAL(firstBeat()),this,SLOT(Test()));
    printLoopList();
}

void interface_c::run()
{
    init();

    exec();
}

void interface_c::keyInput(QKeyEvent *e)
{



    //  qDebug()<<e->key();
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
        pActivePlayPort = pRight;
        pActiveRecPort = pRec0;










        captureData_s *params;
        params = new captureData_s;
        params->createPlayLoop = true;
        params->length=1500;
        params->pPlayPort=pActivePlayPort;
        params->pPort=pActiveRecPort;
        params->pCaptureLoop = &pActiveRecLoop;

        createInterfaceEvent(pClick,SIGNAL(firstBeat()),EVENT_CAPTURE,(void*)params,false,&pActiveEvent);



        break;




        switch(getCaptureStatus(&pActiveRecLoop))
        {
        case 0:
            startRecord(pActivePlayPort,pActiveRecPort,&pActiveRecLoop,2500);
            break;
        case 1:pActiveRecLoop->destroyLoop();break;
        case 2:qDebug()<<"capture loop not created yet, event in progress";break;
        }






        break;

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




        //   new playback_loop_c(9,pActivePlayPort,-1,true);
        //   new playback_loop_c(8,pActivePlayPort,-1,true);

        break;
    case Qt::Key_3:
        pActiveRecPort = pRec1;
        qDebug()<<"Input"<<pActiveRecPort->id<<"selected";
        break;
    case Qt::Key_4:
        if(!pActivePlayLoop){ pActivePlayLoop = this->firstPlayLoop;}

        if(pActivePlayLoop)
        {
            if(pActivePlayLoop->pPrevLoop) {pActivePlayLoop = pActivePlayLoop->pPrevLoop;}
            else {
                playbackListMutex.lock();
                pActivePlayLoop = findLastPlaybackLoop();
                playbackListMutex.unlock();
            }
        }

        if(pActivePlayLoop) qDebug()<<"play loop #"<<pActivePlayLoop->id<<"selected";
        else qDebug()<<"no play loop available";


        break;//
    case Qt::Key_5:
        if(pActivePlayLoop)
        {
            if((pActivePlayLoop->status == IDLE)||(pActivePlayLoop->status == SILENT))  {pActivePlayLoop->play();qDebug()<<"loop"<<pActivePlayLoop->id<<"play";}
            else if(pActivePlayLoop->status == PLAY) {pActivePlayLoop->pause();qDebug()<<"loop"<<pActivePlayLoop->id<<"paused";}

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
    case Qt::Key_8:
        if(pActivePlayLoop)
        {
            if((pActivePlayLoop->status == IDLE)||(pActivePlayLoop->status == SILENT)) {pActivePlayLoop->play();qDebug()<<"loop"<<pActivePlayLoop->id<<"play";}
            else if(pActivePlayLoop->status == PLAY) {pActivePlayLoop->pause();pActivePlayLoop->rewind();qDebug()<<"loop"<<pActivePlayLoop->id<<"stopped";}

        }
        else
            qDebug()<<"no playloop active";

        break;
    case Qt::Key_9:qDebug()<<"Active playback loops count:"<<getPlayLoopsCount();
        qDebug()<<"Active capture loops count:"<<getCaptureLoopsCount();
        qDebug()<<"Active events count:"<<getEventsCount();
        printLoopList();
        break;
    case Qt::Key_Enter:

        parent->close();


        break;


    case Qt::Key_Minus:emit setTempo(pClick->getTempo()-10);break;
    case Qt::Key_Asterisk:emit setTempo(pClick->getTempo()+10);break;

    case Qt::Key_Plus:showPlayLoops();break;

    case Qt::Key_Backspace:

        this->removeAllEvents();qDebug()<<"All events deleted";

        this->removeAllPlaybackLoops();qDebug()<<"All playback loops deleted";


        break;
    case Qt::Key_Slash:
        pClick->stopstart();

        break;
    }


}

void interface_c::Afficher(QString txt)
{
    qDebug()<<txt;

}






void interface_c::Test(void)
{

    pActivePlayPort = pRight;
    pActiveRecPort = pRec0;
    startRecord(pActivePlayPort,pActiveRecPort,&pActiveRecLoop,5000);
}
