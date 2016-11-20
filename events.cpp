#include "events.h"
#include "interface.h"
#include "capture_loop_c.h"
#include "capture_port_c.h"
#include "playback_loop_c.h"
#include "playback_port_c.h"

#include "qdebug.h"





interfaceEvent_c::interfaceEvent_c(const QObject * sender,const char * signal,interfaceEvent_c *pPrevEvent, int eventType, void *param, interface_c *interface):sender(sender),signal(signal),pPrevEvent(pPrevEvent),eventType(eventType),interface(interface)
{
qDebug()<<"create"<<eventType;

    pNextEvent = NULL;
    if(interface->firstEvent == NULL) interface->firstEvent = this;

    switch(eventType)
    {
    case EVENT_CAPTURE:
        captureData *nuData;
        nuData = new(captureData);
        captureData *inData;
        inData = (captureData*)param;
        *nuData = *inData;
        data = (void*)nuData;
        break;

    case EVENT_PLAY:
        playData *nuData2;
        nuData2 = new(playData);
        playData *inData2;
        inData2 = (playData*)param;
        *nuData2 = *inData2;
        data = (void*)nuData2;
    break;
    default:
        return;break;
    }


    if(!connect(sender,signal,this,SLOT(eventProcess()))) qDebug()<<"event connection failure";




}




void interfaceEvent_c::eventProcess() //launch the actual event
{

    switch(eventType)
    {
        case EVENT_CAPTURE:

            captureData *params;
            params = (captureData*)data;
            new capture_loop_c(interface->generateNewId(),params->pPort,params->length,params->createPlayLoop,params->pPlayPort);

            break;

        case EVENT_PLAY:
            playData *params2;
            params2 = (playData*)data;
            new playback_loop_c(params2->id,params2->pPlayPort,params2->length,params2->autoplay);
            break;


        default:return;break;

    }



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
