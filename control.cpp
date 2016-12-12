#include "control.h"
#include "config_file.h"
#include "click_c.h"
#include "capture_loop_c.h"
#include "events.h"

//CLASS CONTROL


control_c::control_c(interface_c* interface,QString control, void (interface_c::*function)() ):
    interface(interface),
    control(control),
    exec_void(function),
    type(VOID)
{}
control_c::control_c(interface_c* interface,QString control, void (interface_c::*function)(QString), QString param ):
    interface(interface),
    control(control),
    exec_qstring(function),
    param_qstring(param),
    type(QSTRING)
{}
control_c::control_c(interface_c* interface,QString control, void (interface_c::*function)(int), int param ):
    interface(interface),
    control(control),
    exec_int(function),
    param_int(param),
    type(INT)
{}




void control_c::exec()
{
    switch(type)
    {
    case VOID:(interface->*(exec_void))();break;
    case INT: (interface->*(exec_int))(param_int);break;
    case QSTRING: (interface->*(exec_qstring))(param_qstring);break;
    }
}





//Class controlList
controlList_c::controlList_c(QString key):key(key)
{
    nextAction = 0;
}

void controlList_c::addElement(control_c* pNuControl)
{
    list.push_back(pNuControl);
}
void controlList_c::exec(void)
{
    unsigned int i;
    for (i = nextAction;i<list.size();i++)
    {
        list[i]->exec();
        if(list[i]->control.contains("Wait for next control")) break;

    }

    nextAction = i+1;
    if(nextAction >= list.size()) nextAction = 0;





}

QString controlList_c::getKey() const
{
    return key;
}



void interface_c::registerControl(QString key,void (interface_c::*function)())
{
    controlId *ctl = new controlId();
    ctl->exec_void=function;
    ctl->key="["+QString::number(controlList.size())+"]"+key;
    ctl->type=VOID;
    controlList.push_back(ctl);
}
void interface_c::registerControl(QString key,void (interface_c::*function)(int param))
{
    controlId *ctl = new controlId();
    ctl->exec_int=function;
    ctl->key="["+QString::number(controlList.size())+"]"+key;
    ctl->type=INT;
    controlList.push_back(ctl);
}
void interface_c::registerControl(QString key,void (interface_c::*function)(QString param))
{
    controlId *ctl = new controlId();
    ctl->exec_qstring=function;
    ctl->key="["+QString::number(controlList.size())+"]"+key;
    ctl->type=QSTRING;
    controlList.push_back(ctl);
}

controlId* interface_c::identifyControl(QString control)
{
    for (auto &pControl : controlList)
    {
        if(control.contains(pControl->key, Qt::CaseSensitive))
        {
            return pControl;
            break;
        }


    }

    return NULL;
}

int interface_c::getControlParamInt(QString control)
{
    for (auto &pControl : controlList)
    {
        if(control.contains(pControl->key, Qt::CaseSensitive))
        {
            control.remove(pControl->key);
            control.remove(" ");
            bool test= true;
            int result = control.toInt(&test);
            if (!test) result = 0;

            return result;

            break;
        }


    }

    return 0;
}

QString interface_c::getControlParamQString(QString control)
{
    for (auto &pControl : controlList)
    {
        if(control.contains(pControl->key, Qt::CaseSensitive))
        {
            control.remove(pControl->key);
            control.remove(" ");
            return control;

            break;
        }


    }

    return "";
}

void interface_c::makeActiveControlsList(void)
{

    //first let us clean up the list
    for (auto &pControl : controlLists) delete pControl;
    controlLists.clear();

    QStringList controlList,controlListKeys;

    fileGetControlKeyList(&controlListKeys);
    for (int i = 0;i<controlListKeys.size();i++)
    {

        controlList_c *pControlList = new controlList_c(controlListKeys[i]);

        controlLists.push_back(pControlList);

        fileGetControl(controlListKeys[i],&controlList);

        for(int j = 0;j<controlList.size();j++)
        {
            controlId *Id = new controlId;
            Id = identifyControl(controlList[j]);

            if(Id)
            {

                switch(Id->type)
                {

                case VOID:
                {
                    control_c *pControl;
                    pControl = new control_c(this,Id->key,Id->exec_void);
                    pControlList->addElement(pControl);
                    break;
                }
                case INT:
                {
                    control_c *pControl;

                    pControl = new control_c(this,Id->key,Id->exec_int,getControlParamInt(controlList[j]));
                    pControlList->addElement(pControl);
                    break;
                }
                case QSTRING:
                {
                    control_c *pControl;

                    pControl = new control_c(this,Id->key,Id->exec_qstring,getControlParamQString(controlList[j]));
                    pControlList->addElement(pControl);
                    break;
                }

                }

            }
            else qDebug()<<controlList[j]<<"unknown control";

        }

    }

}

void interface_c::activateControl(QString key)
{
    for (auto &pControls : controlLists)
    {
        if(pControls->getKey() == key)
        {
            pControls->exec();
        }
    }
}





//control functions

void interface_c::waitForNextKey(void)
{
    // qDebug()<<"wait For next key";
}

void interface_c::startstopClick(void)
{
    if(pClick)
    {
        pClick->stopstart();
    }
}
void interface_c::unselectAllPlaybacks()
{
    selectedPlaybackPortsList.clear();
    updatePortsConsole();
}
void interface_c::selectAllPlaybacks()
{
    selectedPlaybackPortsList = playbackPortsList;
    updatePortsConsole();
}
void interface_c::unselectPlayback(int channel)
{
    playback_port_c *pPort = findPlaybackPortByChannel(channel);
    if(!pPort) return;

    auto entry = std::find(selectedPlaybackPortsList.begin(),selectedPlaybackPortsList.end(),pPort);
    if(entry != selectedPlaybackPortsList.end())
        selectedPlaybackPortsList.erase(entry);
    updatePortsConsole();
}
void interface_c::selectPlayback(int channel)
{

    playback_port_c *pPort = findPlaybackPortByChannel(channel);

    if(!pPort)
        return;

    if ( std::find(selectedPlaybackPortsList.begin(), selectedPlaybackPortsList.end(), pPort) != selectedPlaybackPortsList.end() )
        return;

    selectedPlaybackPortsList.push_back(pPort);
    updatePortsConsole();
}

void interface_c::selectCapture(int channel)
{
    qDebug()<<"select capture"<<channel;
    capture_port_c *pPort = findCapturePortByChannel(channel);
    if(!pPort)
        return;
    selectedCapturePortsList.clear();
    selectedCapturePortsList.push_back(pPort);
    updatePortsConsole();

}
void interface_c::selectNextCapture(void)
{

    if(capturePortsList.size()==0) return;
    if(selectedCapturePortsList.size()==0)
    {
        selectedCapturePortsList.push_back(capturePortsList[0]);
        updatePortsConsole();
        return;

    }



    for(unsigned int i = 0;i<capturePortsList.size();i++)
    {
        if(capturePortsList[i] == selectedCapturePortsList[0])
        {
            if(i+1 < capturePortsList.size())
            {
                selectedCapturePortsList[0] = capturePortsList[i+1];
                updatePortsConsole();
                return;
            }
            else
            {
                selectedCapturePortsList[0] = capturePortsList[0];
                updatePortsConsole();
                return;
            }

        }

    }



}

void interface_c::createCapture(int desiredId)
{
    qDebug()<<"start capture"<<desiredId;
    if(!isIdFree(desiredId)) {qDebug()<<"Id"<<desiredId<<"already busy";return;}

    if(selectedCapturePortsList.size() == 0) {qDebug()<<"error: no capture port selected";return;}
    // if(selectedPlaybackPortsList.size() == 0) {qDebug()<<"error: no playback port selected";return;}

    if(synchroMode == CLICKSYNC)
    {

        if(pClick->getBeat()<1) //capture was launched less than a beat too late, we are not going to wait till the next bar...
        {
            new capture_loop_c(desiredId,selectedCapturePortsList[0],0,false,selectedPlaybackPortsList,pClick->getTime());

        }

        else
        {

            captureData_s *params = new captureData_s;
            params->createPlayLoop = false;
            params->length=0;
            params->pPlayPorts = selectedPlaybackPortsList;
            params->pPort=selectedCapturePortsList[0];
            params->pCaptureLoop = NULL;
            params->id = desiredId;

            createInterfaceEvent(pClick,SIGNAL(firstBeat()),EVENT_CAPTURE,(void*)params,false,NULL);
        }
    }

}
void interface_c::createCaptureAndPlay(int desiredId)
{

    qDebug()<<"captureandplay Id"<<desiredId;

    if(!isIdFree(desiredId)) {qDebug()<<"Id"<<desiredId<<"already busy";return;}

    if(selectedCapturePortsList.size() == 0) {qDebug()<<"error: no capture port selected";return;}
    if(selectedPlaybackPortsList.size() == 0) {qDebug()<<"error: no playback port selected";return;}

    if(synchroMode == CLICKSYNC)
    {

        if(pClick->getBeat()<1) //capture was launched less than a beat too late, we are not going to wait till the next bar...
        {
            new capture_loop_c(desiredId,selectedCapturePortsList[0],0,true,selectedPlaybackPortsList,pClick->getTime());

        }

        else
        {

            captureData_s *params = new captureData_s;
            params->createPlayLoop = true;
            params->length=0;
            params->pPlayPorts = selectedPlaybackPortsList;
            params->pPort=selectedCapturePortsList[0];
            params->pCaptureLoop = NULL;
            params->id = desiredId;

            createInterfaceEvent(pClick,SIGNAL(firstBeat()),EVENT_CAPTURE,(void*)params,false,NULL);
        }
    }

}
void interface_c::stopCapture(int id)
{
    capture_loop_c *pLoop = findCaptureLoopById(id);

    if(pLoop)
        pLoop->destroyLoop();
}

void interface_c::startLoop(int id)
{
    qDebug()<<"start loop"<<id;

    if(selectedPlaybackPortsList.size() == 0) {qDebug()<<"error: no playback port selected";return;}

    if(findPlayLoopById(id)) {qDebug()<<"error: loop already playing";return;}


    if(synchroMode == NOSYNC)
    {


        //  pPlayLoop = new playback_loop_c(id,pPlayPorts,playPortsCount,-1,NOSYNC,IDLE,pPort->interface);//by default loop
    }
    else if(synchroMode == CLICKSYNC)
    {




        if(pClick->getBeat()<1) //capture was launched less than a beat too late, we are not going to wait till the next bar...
        {
            new playback_loop_c(id,selectedPlaybackPortsList,0,CLICKSYNC,PLAY,this,pClick->getTime());

        }

        else
        {


            playData_s *param;
            param = new playData_s;
            param->id = id;
            param->length = 0;
            param->pPlayPorts = selectedPlaybackPortsList;
            param->skipevent = 0;
            param->status = PLAY;
            param->syncMode = CLICKSYNC;
            param->pCaptureLoop = NULL;

            emit createInterfaceEvent(pClick,SIGNAL(firstBeat()),EVENT_CREATE_PLAY,(void*)param,false,NULL);


        }
    }



}
void interface_c::stopLoop(int id)
{
    playback_loop_c *pLoop = findPlayLoopById(id);
    if(pLoop)
        pLoop->destroy();
}
void interface_c::startstopLoop(int id)
{
    playback_loop_c *pLoop = findPlayLoopById(id);
    if(pLoop)
        pLoop->destroy();
    else
    {

        startLoop(id);


    }
}
void interface_c::stopAllLoops(void)
{
    qDebug()<<"stop all loops";

    playback_loop_c *pLoop = firstPlayLoop;
    playback_loop_c *pLoop2 = NULL;
    while(pLoop != NULL)
    {

        pLoop2 = pLoop->pNextLoop;

        pLoop->destroy();

        pLoop = pLoop2;

    }
}
void interface_c::stopSelectedLoop(void)
{
    if(selectedPlayLoop)
    {
        selectedPlayLoop->destroy();
    }


}

void interface_c::muteunmuteLoop(int id)
{
    playback_loop_c *pLoop = findPlayLoopById(id);
    if(pLoop)
    {
        if(pLoop->status==PLAY)
            pLoop->pause();
        else
            pLoop->play();
    }
}
void interface_c::muteunmuteAllLoops(void)
{
    if(getAllMute()) setAllMute(false);
    else setAllMute(true);
}

void interface_c::moveLoop(int id)
{

    playback_loop_c *pLoop = findPlayLoopById(id);
    if(pLoop)
        pLoop->moveToPort(selectedPlaybackPortsList);

}
void interface_c::moveSelectedLoop()
{
    if(selectedPlayLoop)
        selectedPlayLoop->moveToPort(selectedPlaybackPortsList);
}

void interface_c::selectNextLoop(void)
{
    if(!selectedPlayLoop) selectedPlayLoop = firstPlayLoop;
    else
    {

        if(selectedPlayLoop->pNextLoop)
            selectedPlayLoop = selectedPlayLoop->pNextLoop;
        else
            selectedPlayLoop = firstPlayLoop;
    }
    printPlaybackLoopList();

}
void interface_c::selectPrevLoop(void)
{
    if(!selectedPlayLoop) selectedPlayLoop = firstPlayLoop;
    else
    {


        if(selectedPlayLoop->pPrevLoop)
            selectedPlayLoop = selectedPlayLoop->pPrevLoop;
        else
            selectedPlayLoop = findLastPlaybackLoop();

    }
    printPlaybackLoopList();
}

//TODO




void interface_c::createControls(void)
{
    registerControl("Wait for next control",&interface_c::waitForNextKey);

    registerControl("Start capture",&interface_c::createCapture);
    registerControl("Start capture autoplay",&interface_c::createCaptureAndPlay);
    registerControl("Move loop to ports",&interface_c::moveLoop);

    registerControl("Stop capture",&interface_c::stopCapture);

    registerControl("Select playback",&interface_c::selectPlayback);
    registerControl("Unselect playback",&interface_c::unselectPlayback);
    registerControl("Select all playbacks",&interface_c::selectAllPlaybacks);
    registerControl("Unselect all playbacks",&interface_c::unselectAllPlaybacks);


    registerControl("Select capture",&interface_c::selectCapture);
    registerControl("Select next capture",&interface_c::selectNextCapture);


    registerControl("Start/stop loop",&interface_c::startstopLoop);
    registerControl("Start loop",&interface_c::startLoop);
    registerControl("Stop loop",&interface_c::stopLoop);
    registerControl("Stop all loops",&interface_c::stopAllLoops);



    registerControl("Mute/unmute all loops",&interface_c::muteunmuteAllLoops);
    registerControl("Mute/unmute loop",&interface_c::muteunmuteLoop);
    registerControl("Start/stop click",&interface_c::startstopClick);

    registerControl("Select previous loop",&interface_c::selectPrevLoop);
    registerControl("Select next loop",&interface_c::selectNextLoop);

    registerControl("Stop selected loop",&interface_c::stopSelectedLoop);
    registerControl("Move selected loop",&interface_c::moveSelectedLoop);


}



