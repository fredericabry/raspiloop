#include "capture_loop_c.h"
#include "playback_loop_c.h"
#include "interface.h"
#include "qdebug.h"
#include "click_c.h"
#include "events.h"
#include "alsa_playback_device.h"
#include "alsa_capture_device.h"
#include "config_file.h"
#include "dialog_newcontrol.h"
#include "control.h"




interfaceEvent_c *pActiveEvent;




interface_c::interface_c(MainWindow *parent):parent(parent)
{
    firstPlayLoop = NULL;//no playback loop at this point
    firstCaptureLoop = NULL;//no capture loop at this point
    firstEvent = NULL;//no event at this point

}

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


    eventListMutex.lock();
    interfaceEvent_c *pEvent = firstEvent;

    if(pEvent != NULL)
    {

        if(pEvent->eventType==EVENT_CREATE_PLAY)
        {
            if(pEvent->playData->id >= id) id = pEvent->playData->id+1;
        }

        while(pEvent->pNextEvent!=NULL)
        {
            pEvent=pEvent->pNextEvent;
            if(pEvent->eventType==EVENT_CREATE_PLAY)
            {
                if(pEvent->playData->id >= id) id = pEvent->playData->id+1;
            }
        }

    }



    eventListMutex.unlock();



    return id;

}


bool interface_c::isIdFree(int id)
{


    playbackListMutex.lock();
    playback_loop_c *pPlayLoop = firstPlayLoop;
    if(pPlayLoop != NULL)
    {
        if(pPlayLoop->id == id) {playbackListMutex.unlock();return false;}
        while(pPlayLoop->pNextLoop!=NULL)
        {
            pPlayLoop=pPlayLoop->pNextLoop;
            if(pPlayLoop->id == id) {playbackListMutex.unlock();return false;}
        }
    }
    playbackListMutex.unlock();
    captureListMutex.lock();
    capture_loop_c *pCaptureLoop = firstCaptureLoop;
    if(pCaptureLoop != NULL)
    {
        if(pCaptureLoop->id == id) {captureListMutex.unlock();return false;;}
        while(pCaptureLoop->pNextLoop!=NULL)
        {
            pCaptureLoop=pCaptureLoop->pNextLoop;
            if(pCaptureLoop->id == id) {captureListMutex.unlock();return false;}
        }
    }
    captureListMutex.unlock();


    eventListMutex.lock();
    interfaceEvent_c *pEvent = firstEvent;

    if(pEvent != NULL)
    {

        if(pEvent->eventType==EVENT_CREATE_PLAY)
        {
            if(pEvent->playData->id == id) {eventListMutex.unlock();return false;}
        }
        else if(pEvent->eventType==EVENT_CAPTURE)
        {
            if(pEvent->captureData->id == id) {eventListMutex.unlock();return false;}
        }

        while(pEvent->pNextEvent!=NULL)
        {
            pEvent=pEvent->pNextEvent;
            if(pEvent->eventType==EVENT_CREATE_PLAY)
            {
                if(pEvent->playData->id == id) {eventListMutex.unlock();return false;}
            }
            else if(pEvent->eventType==EVENT_CAPTURE)
            {
                if(pEvent->playData->id == id) {eventListMutex.unlock();return false;}
            }
        }

    }



    eventListMutex.unlock();


    return true;
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

capture_loop_c* interface_c::findCaptureLoopById(int id)
{
    captureListMutex.lock();
    capture_loop_c *pLoop = firstCaptureLoop;
    while(pLoop != NULL)
    {
        if(pLoop->id == id) {captureListMutex.unlock();return pLoop;}
        pLoop = pLoop->pNextLoop;
    }
    captureListMutex.unlock();
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

void interface_c::removeRestartEvents(int id)
{

    interfaceEvent_c* pEvent = firstEvent;
    std::vector<interfaceEvent_c*> eventsToDestroy;

    if(pEvent == NULL)
    {
        return;
    }

    if(pEvent->eventType ==  EVENT_PLAY_RESTART)
        if(pEvent->restartplayData->id == id)
            eventsToDestroy.push_back(pEvent);

    while(pEvent->pNextEvent)
    {
        pEvent = pEvent->pNextEvent;

        if(pEvent->eventType ==  EVENT_PLAY_RESTART)
            if(pEvent->restartplayData->id == id)
                eventsToDestroy.push_back(pEvent);
    }


    for(auto pEvent:eventsToDestroy)
        pEvent->destroy();

    eventsToDestroy.clear();

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
    pClick->stopstart();

}

void interface_c::moveClick(std::vector<playback_port_c*> pPorts)
{
    if(pClick)
    {
        pClick->pPorts.clear();
        pClick->pPorts = pPorts;
    }
}

QString interface_c::statusToString(status_t status)
{
    switch(status)
    {
    case IDLE:return "stopped";break;
    case PLAY:return "playing";break;
    case SILENT:return "silent";break;
    default:return "unknown";break;
    }
}

void interface_c::printPlaybackLoopList(void)
{
    playback_loop_c *pLoop = this->firstPlayLoop;
    QString txt = "";


    if(getPlayLoopsCount() <= 0) txt="no loop";
    else
    {

        if(pLoop)
        {
            txt+="Loop #"+QString::number(pLoop->id)+":"+statusToString(pLoop->status);

            if(pLoop->pPorts.size()==0)
                txt+=" - no port";
            else if(pLoop->pPorts.size()==1)
                txt+=" - port:" +QString::number(pLoop->pPorts[0]->channel);
            else
            {
                txt+=" - ports:";
                for (auto &pPort : pLoop->pPorts) txt+= QString::number(pPort->channel)+" ";

            }


            txt+=" - length:"+QString::number(pLoop->barstoplay)+"bars"+"\n";

        }

        while(pLoop->pNextLoop)
        {
            pLoop = pLoop->pNextLoop;
            if(pLoop)    {
                txt+="Loop #"+QString::number(pLoop->id)+":"+statusToString(pLoop->status);

                if(pLoop->pPorts.size()==0)
                    txt+=" - no port";
                else if(pLoop->pPorts.size()==1)
                    txt+=" - port:" +QString::number(pLoop->pPorts[0]->channel);
                else
                {
                    txt+=" - ports:";
                    for (auto &pPort : pLoop->pPorts) txt+= QString::number(pPort->channel)+" ";

                }


                txt+=" - length:"+QString::number(pLoop->barstoplay)+"bars"+"\n";
            }

        }
    }

    emit playbackLoopList(txt);

}



void interface_c::printCaptureLoopList(void)
{
    capture_loop_c *pLoop = this->firstCaptureLoop;
    QString txt = "";


    if(getCaptureLoopsCount() <= 0) txt="no loop";
    else
    {

        if(pLoop)
        {
            txt+="Loop #"+QString::number(pLoop->id);

            if(pLoop->pPort)
                txt+=" - port:" +QString::number(pLoop->pPort->channel)+"\n";
            else
            {
              txt+=" - no port\n";
            }




        }

        while(pLoop->pNextLoop)
        {
            pLoop = pLoop->pNextLoop;
            if(pLoop)    {
                txt+="Loop #"+QString::number(pLoop->id);

                if(pLoop->pPort)
                    txt+=" - port:" +QString::number(pLoop->pPort->channel)+"\n ";
                else
                {
                  txt+=" - no port\n";
                }



            }

        }
    }

    emit captureLoopList(txt);

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


    if(pEvent == NULL) {return 0;}
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




void interface_c::init(void)
{


    playbackPortsCount = 0;
    capturePortsCount = 0;

    QStringList device;
    extractParameter(KEYWORD_CAPTURE_LIST, &device);

    for(int i =0;i<device.size();i++)
    {
        bool deviceCreated = false;
        alsa_capture_device *pCaptureDevice = new alsa_capture_device(device[i],2,RATE,this,&deviceCreated);
        if(deviceCreated)
        {
        captureDevicesList.push_back(pCaptureDevice);
        for(int j=0;j<pCaptureDevice->capture_channels;j++)
            capturePortsList.push_back(pCaptureDevice->alsa_capture_port_by_num(j));
        }
    }


    extractParameter(KEYWORD_PLAYBACK_LIST, &device);
    for(int i =0;i<device.size();i++)
    {
        bool deviceCreated = false;
        alsa_playback_device *pPlaybackDevice = new alsa_playback_device(device[i],2,RATE,this,&deviceCreated);
        if(deviceCreated)
        {
        playbackDevicesList.push_back(pPlaybackDevice);
        for(int j=0;j<pPlaybackDevice->playback_channels;j++)
            playbackPortsList.push_back(pPlaybackDevice->alsa_playback_port_by_num(j));
        }
    }





    synchroMode = CLICKSYNC;

    isRecording = false;



    QStringList portNamesFile;
    std::vector <playback_port_c *> pClickPorts;
    extractParameter(KEYWORD_CLICK_PORTS, &portNamesFile);

    for(int i =0;i<portNamesFile.size();i++)
    {


        if((portNamesFile[i].toInt()> 0)&&(portNamesFile[i].toInt()<= (signed)playbackPortsList.size()))
            pClickPorts.push_back(playbackPortsList[portNamesFile[i].toInt()-1]);

    }


    //get mix strategies
    QStringList mix,mix_preset;
    extractParameter(KEYWORD_MIX_STRATEGY, &mix);
    extractParameter(KEYWORD_MIX_PRESET_NUMBER, &mix_preset);
    if(mix.size()>0) //we only use [0] value
    {
        if(mix[0] == MIX_STRATEGY_PRESET)
        {
            setMixStrategy(PRESET);
        }
        else if(mix[0] == MIX_STRATEGY_AUTO)
        {
            setMixStrategy(AUTO);
        }
    }


    bool test = true;

    if(mix_preset.size()>0)
    {
        setMixLoopNumber(mix_preset[0].toInt(&test));
        if(!test) setMixLoopNumber(1);//error
    }
    else setMixLoopNumber(1);
















    pClick = new click_c(120,pClickPorts,IDLE,this,parent);
    clickStatus=false;
    connect(this,SIGNAL(setTempo(int)),pClick,SLOT(setTempo(int)));
    connect(this,SIGNAL(playbackLoopList(QString)),parent,SLOT(setPlaybackLoopList(QString)));
    connect(this,SIGNAL(captureLoopList(QString)),parent,SLOT(setCaptureLoopList(QString)));


    connect(this,SIGNAL(sendPlaybackConsole(QString)),parent,SLOT(setPlaybackConsole(QString)));
    connect(this,SIGNAL(sendCaptureConsole(QString)),parent,SLOT(setCaptureConsole(QString)));






    telapsed.start();
    //connect(pClick,SIGNAL(firstBeat()),this,SLOT(Test()));
    printPlaybackLoopList();
    printCaptureLoopList();


    createControls();


    makeActiveControlsList();

    updatePortsConsole();


    setAllMute(false);


}

void interface_c::run()
{
    init();
    exec();
}

void interface_c::keyInput(QKeyEvent *e)
{


    activateControl("KEY_"+QKeySequence(e->key()).toString());


}

void interface_c::Afficher(QString txt)
{
    qDebug()<<txt;

}

mixStrategies interface_c::getMixStrategy() const
{
    return mixStrategy;
}

void interface_c::setMixStrategy(mixStrategies value)
{
    mixStrategy = value;
}

unsigned int interface_c::getMixLoopNumber()
{
    if(mixLoopNumber==0)
        setMixLoopNumber(1);

    return mixLoopNumber;
}

void interface_c::setMixLoopNumber(unsigned int value)
{
    mixLoopNumber = value;
}

bool interface_c::getAllMute() const
{
    return allMute;
}

void interface_c::setAllMute(bool value)
{
    allMute = value;
}


capture_port_c* interface_c::findCapturePortByChannel(int channel)
{

    for (auto &pPort : capturePortsList )
    {
        if(pPort->channel == channel) return pPort;
    }

    return NULL;
}

playback_port_c* interface_c::findPlaybackPortByChannel(int channel)
{
    for (auto &pPort : playbackPortsList )
    {
        if(pPort->channel == channel) return pPort;
    }

    return NULL;

}


void interface_c::updatePortsConsole(void)
{

    QString txt;




    for (auto &pPort : selectedPlaybackPortsList)
        txt.append(QString::number(pPort->channel)+" ");

    emit sendPlaybackConsole(txt);
    txt.clear();

    for (auto &pPort : selectedCapturePortsList)
        txt.append(QString::number(pPort->channel)+" ");


    emit sendCaptureConsole(txt);




}


void interface_c::destroy(void)
{
    for (auto &pPlaybackDevice : playbackDevicesList) pPlaybackDevice->alsa_cleanup_playback();
    for (auto &pCaptureDevice : captureDevicesList) pCaptureDevice->alsa_cleanup_capture();
    for (auto &pControl : controlLists) delete pControl;


}


