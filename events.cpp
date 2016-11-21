#include "events.h"
#include "interface.h"
#include "capture_loop_c.h"
#include "capture_port_c.h"
#include "playback_loop_c.h"
#include "playback_port_c.h"

#include "qdebug.h"





interfaceEvent_c::interfaceEvent_c(const QObject * sender,const char * signal,interfaceEvent_c *pPrevEvent, int eventType, void *param, interface_c *interface,bool repeat):sender(sender),signal(signal),pPrevEvent(pPrevEvent),eventType(eventType),interface(interface),repeat(repeat)
{

    count = 0;

    pNextEvent = NULL;
    if(interface->firstEvent == NULL) interface->firstEvent = this;
    else
    {
    pPrevEvent->pNextEvent = this;
    }



    switch(eventType)
    {
    case EVENT_CAPTURE:
        captureData_s *captureData;
        captureData= new captureData_s;
        *captureData= (*(captureData_s*)param);
         data = (void*)captureData;
        break;

    case EVENT_CREATE_PLAY:
        playData_s *playData;
        playData = new playData_s;
        *playData = *((playData_s*)param);
        data = (void*)playData;
        break;

    case EVENT_PLAY_RESTART:
        restartplayData_s *restartplayData;
        restartplayData = new(restartplayData_s);
        *restartplayData = *((restartplayData_s*)param);
        data = (void*)restartplayData;
        break;

    default:
        return;break;
    }


    if(!connect(sender,signal,this,SLOT(eventProcess()))) qDebug()<<"event connection failure";




}




void interfaceEvent_c::updateParameters(void *param)
{

    switch(eventType)
    {
    case EVENT_CAPTURE:
        captureData_s *captureData;
        captureData= new captureData_s;
        *captureData= (*(captureData_s*)param);
         data = (void*)captureData;
        break;

    case EVENT_CREATE_PLAY:
        playData_s *playData;
        playData = new playData_s;
        *playData = *((playData_s*)param);
        data = (void*)playData;
        break;

    case EVENT_PLAY_RESTART:
        restartplayData_s *restartplayData;
        restartplayData = new(restartplayData_s);
        *restartplayData = *((restartplayData_s*)param);
        data = (void*)restartplayData;
        break;
    }

}





void interfaceEvent_c::eventProcess() //launch the actual event
{

    bool killEvent = false;

    switch(eventType)
    {
    case EVENT_CAPTURE:

        captureData_s *params;
        params = (captureData_s*)data;
        new capture_loop_c(interface->generateNewId(),params->pPort,params->length,params->createPlayLoop,params->pPlayPort);
        killEvent = true;
        break;

    case EVENT_CREATE_PLAY:

        playData_s *params2;
        params2 = (playData_s*)data;
        if(params2->skipevent>0)
        {
            params2->skipevent--;
            *((playData_s*)data) = *(params2);
            killEvent = false;

        }
        else
        {
            new playback_loop_c(params2->id,params2->pPlayPort,params2->length,params2->syncMode,params2->status);
          //  qDebug()<<"create loop";
            killEvent = true;

        }
        break;

    case EVENT_PLAY_RESTART://signal to rewind and play a loop
         //qDebug()<<"restart";


        restartplayData_s *params3;
        params3 = (restartplayData_s*)data;

        if(count<params3->skipevent)
        {
            count++;
            killEvent = false;
           // qDebug()<<"skip"<<count<<"/"<<params3->skipevent;

        }
        else
        {
            playback_loop_c *pLoop;
            pLoop = interface->findPlayLoopById(params3->id);
            if(pLoop)
            {
                //qDebug()<<"process"<<pLoop->status;;
                pLoop ->rewind();

                pLoop->status = params3->status;
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
    //   qDebug()<<"event killed";

}



void interfaceEvent_c::destroy(void)
{

    //event has been performed, let's remove it from the list

    if(pPrevEvent == NULL) interface->firstEvent = pNextEvent;//it was the first event, let's update this info

    if(pNextEvent == NULL) //it was the last event
    {
        if(pPrevEvent != NULL) pPrevEvent->pNextEvent = NULL; //if it was not the first event
    }
    else
    {
        pNextEvent->pPrevEvent=pPrevEvent;
        if(pPrevEvent != NULL) pPrevEvent->pNextEvent = pNextEvent;
    }

    //we can destroy this class
    delete this;

}

