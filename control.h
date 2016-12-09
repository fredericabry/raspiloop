#ifndef CONTROL_H
#define CONTROL_H
#include "qstring.h"
#include "qdebug.h"
#include "interface.h"

class interface_c;


class control
{
public:
    control(interface_c* interface,QString key,void (interface_c::*function)() );
    control(interface_c* interface,QString key,void (interface_c::*function)(QString) , QString param);
    control(interface_c* interface,QString key,void (interface_c::*function)(int) , int param);

    interface_c* interface;
    QString key;

    void (interface_c::*exec_void)();

    void (interface_c::*exec_qstring)(QString);
    QString param_qstring;

    void (interface_c::*exec_int)(int);
    int param_int;

    type_e  type;
    void exec(void);

};





#endif // CONTROL_H

