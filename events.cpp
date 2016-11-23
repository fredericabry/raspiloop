#include "events.h"
#include "interface.h"
#include "capture_loop_c.h"
#include "capture_port_c.h"
#include "playback_loop_c.h"
#include "playback_port_c.h"

#include "qdebug.h"





interfaceEvent_c::interfaceEvent_c(const QObject * sender,const char * signal,const int eventType, void *param, interface_c *interface,bool repeat):sender(sender),signal(signal),eventType(eventType),interface(interface),repeat(repeat)
{


    addToList();

    count = 0;





    switch(eventType)
    {
    case EVENT_CAPTURE:
        captureData= new captureData_s;
        *captureData= (*(captureData_s*)param);
        break;

    case EVENT_CREATE_PLAY:
        playData = new playData_s;
        *playData = *((playData_s*)param);
        break;

    case EVENT_PLAY_RESTART:
        restartplayData = new(restartplayData_s);
        *restartplayData = *((restartplayData_s*)param);
        break;

    default:
        return;break;
    }


    delete param;

    if(!connect(sender,signal,this,SLOT(eventProcess()))) qDebug()<<"event connection failure";




}

void interfaceEvent_c::addToList(void)
{



    pNextEvent = NULL;//last event created
    pPrevEvent = interface->findLastEvent();


    if(interface->firstEvent == NULL) {interface->firstEvent = this;}
    else if(pPrevEvent)
    {
        pPrevEvent->pNextEvent = this;
    }
    else qDebug()<<"bug event list add";




}

void interfaceEvent_c::removeFromList(void)
{


    if(interface->firstEvent == this) interface->firstEvent = pNextEvent;//it was the first event, let's update this info


    if(pPrevEvent) pPrevEvent->pNextEvent = pNextEvent; //if it was not the first event
    if(pNextEvent) pNextEvent->pPrevEvent=pPrevEvent; //it was not the last event



}

void interfaceEvent_c::updateParameters(void *param)
{

    switch(eventType)
    {
    case EVENT_CAPTURE:
        qDebug()<<"capt";
        *captureData= (*(captureData_s*)param);
        break;

    case EVENT_CREATE_PLAY:
        qDebug()<<"play";
        *playData = *((playData_s*)param);
        break;

    case EVENT_PLAY_RESTART:
        qDebug()<<"rewind";
        *restartplayData = *((restartplayData_s*)param);
        break;
    }

}

void interfaceEvent_c::eventProcess() //launch the actual event
{

    //qDebug()<<"process"<<eventType;
    bool killEvent = false;

    switch(eventType)
    {
    case EVENT_CAPTURE:

        *(captureData->pCaptureLoop) = new capture_loop_c(interface->generateNewId(),captureData->pPort,captureData->length,captureData->createPlayLoop,captureData->pPlayPort,0);
        killEvent = true;

        break;

    case EVENT_CREATE_PLAY:


        if(playData->skipevent>0)
        {
            playData->skipevent--;
            killEvent = false;
        }
        else
        {
            *(playData->pPlayLoop) = new playback_loop_c(playData->id,playData->pPlayPort,playData->length,playData->syncMode,playData->status);

            killEvent = true;
        }

        break;

    case EVENT_PLAY_RESTART://signal to rewind and play a loop
        //qDebug()<<"restart";
        if(count<restartplayData->skipevent)
        {
            count++;
            killEvent = false;
            // qDebug()<<"skip"<<count<<"/"<<params3->skipevent;

        }
        else
        {


            if(restartplayData->pLoop)
            {
                //qDebug()<<"process"<<pLoop->status;;
                restartplayData->pLoop ->rewind();

                //  qDebug()<<"status"<<pLoop->status << params3->status;
                //  pLoop->status = params3->status;
            }


            killEvent = true;
            count = 0;
        }
        break;



    default:return;break;

    }





    if((killEvent)&&(!repeat))
    {
        destroy();
    }


}

interfaceEvent_c::~interfaceEvent_c()
{
    //qDebug()<<"event killed";

}

void interfaceEvent_c::destroy(void)
{

    //event has been performed, let's remove it from the list

    removeFromList();
    //we can destroy this class

    if(!disconnect(sender,signal,this,SLOT(eventProcess()))) qDebug()<<"event disconnection failure";
    //    delete this;
    this->deleteLater();



}

void interfaceEvent_c::test()
{
    qDebug()<<"event ok, type:"<<eventType;

}
