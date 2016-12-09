#include "control.h"



control::control(interface_c* interface,QString key, void (interface_c::*function)() ):
    interface(interface),
    key(key),
    exec_void(function),
    type(VOID)
{}
control::control(interface_c* interface,QString key, void (interface_c::*function)(QString), QString param ):
    interface(interface),
    key(key),
    exec_qstring(function),
    param_qstring(param),
    type(QSTRING)
{}
control::control(interface_c* interface,QString key, void (interface_c::*function)(int), int param ):
    interface(interface),
    key(key),
    exec_int(function),
    param_int(param),
    type(INT)
{}




void control::exec()
{
    switch(type)
    {
    case VOID:(interface->*(exec_void))();break;
    case INT: (interface->*(exec_int))(param_int);break;
    case QSTRING: (interface->*(exec_qstring))(param_qstring);break;
    }
}


void interface_c::createCapture(int desiredId)
{
}
void interface_c::stopCapture(int Id)
{
}

void interface_c::unselectAllPlaybacks()
{

}
void interface_c::selectAllPlaybacks()
{

}

void interface_c::selectPlayback(int channel)
{
}
void interface_c::unselectPlayback(int channel)
{
}
void interface_c::selectCapture(int channel)
{
}
void interface_c::unselectCapture(int channel)
{
}

void interface_c::unselectAllCaptures()
{
}
void interface_c::selectAllCaptures()
{
}
void interface_c::startstopLoop(int id)
{
}
void interface_c::startLoop(int id)
{
}
void interface_c::stopLoop(int id)
{
}
void interface_c::stopAllLoops(void)
{
}
void interface_c::muteunmuteLoop(int id)
{
}
void interface_c::muteunmuteAllLoops(void)
{
}
void interface_c::waitForNextControl(void)
{
}






void interface_c::registerControl(QString key,void (interface_c::*function)())
{
    controlId *ctl = new controlId();
    ctl->exec_void=function;
    ctl->key=key;
    ctl->type=VOID;
    controlList.push_back(ctl);
}

void interface_c::registerControl(QString key,void (interface_c::*function)(int param))
{
    controlId *ctl = new controlId();
    ctl->exec_int=function;
    ctl->key=key;
    ctl->type=INT;
    controlList.push_back(ctl);
}

void interface_c::registerControl(QString key,void (interface_c::*function)(QString param))
{
    controlId *ctl = new controlId();
    ctl->exec_qstring=function;
    ctl->key=key;
    ctl->type=QSTRING;
    controlList.push_back(ctl);
}






void interface_c::createControls(void)
{
    registerControl("Wait for next control",&interface_c::waitForNextControl);
    registerControl("Start capture",&interface_c::createCapture);
    registerControl("Stop capture",&interface_c::stopCapture);
    registerControl("Select playback",&interface_c::selectPlayback);
    registerControl("Unselect playback",&interface_c::unselectPlayback);
    registerControl("Select all playbacks",&interface_c::selectAllPlaybacks);
    registerControl("Unselect all playbacks",&interface_c::unselectAllPlaybacks);
    registerControl("Select capture",&interface_c::selectCapture);
    registerControl("Unselect capture",&interface_c::unselectCapture);
    registerControl("Select all captures",&interface_c::selectAllCaptures);
    registerControl("Unselect all captures",&interface_c::unselectAllCaptures);
    registerControl("Start/stop loop",&interface_c::startstopLoop);
    registerControl("Start loop",&interface_c::startLoop);
    registerControl("Stop loop",&interface_c::stopLoop);
    registerControl("Stop all loops",&interface_c::stopAllLoops);
    registerControl("Mute all loops",&interface_c::muteunmuteAllLoops);
    registerControl("Mute/unmute loop",&interface_c::muteunmuteLoop);

}
