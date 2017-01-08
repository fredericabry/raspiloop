#ifndef CONTROL_H
#define CONTROL_H
#include "qstring.h"
#include "qdebug.h"
#include "interface.h"



#define CONTROL_MARKER "_"

class interface_c;







class control_c
{
public:

    control_c(interface_c* interface,QString control,void (interface_c::*function)() );
    control_c(interface_c* interface,QString control,void (interface_c::*function)(QString) , QString param);
    control_c(interface_c* interface,QString control,void (interface_c::*function)(int) , int param);
    interface_c* interface;
    QString control;
    void (interface_c::*exec_void)();
    void (interface_c::*exec_qstring)(QString);
    QString param_qstring;
    void (interface_c::*exec_int)(int);
    int param_int;
    controlType_e  type;
    void exec(void);

};



class controlList_c
{
public:
    controlList_c(QString key);
    void addElement(control_c* pNuControl);
    void exec();
    QString getKey() const;
    int CCvalue;//if a CC midi msg is used to launch this control list, then here is stored the value associated with the message
private:
    unsigned int nextAction;
    QString key;
    std::vector<control_c*> list;

};



#endif // CONTROL_H

