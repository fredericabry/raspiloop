#ifndef EVENTS_H
#define EVENTS_H

#define EVENT_CAPTURE 1
#define EVENT_CREATE_PLAY 2
#define EVENT_PLAY_RESTART 3


#include <QMainWindow>
#include "interface.h"
#include "playback_loop_c.h"

class playback_loop_c;
class capture_loop_c;
class capture_port_c;
class playback_port_c;
class interface_c;


struct captureData_s
{
    capture_port_c *pPort;
    long length;
    bool createPlayLoop;
    playback_port_c *pPlayPort;
};




struct playData_s
{
    int id;
    playback_port_c *pPlayPort;
    long length;
    int skipevent;//number of event call to skip before actually processing the event
    syncoptions syncMode;
    status_t status;
    playback_loop_c **pPlayLoop;//where the capture loop stores the pointer to the playloop


};

struct restartplayData_s
{
    int id;
    int skipevent;
    status_t status;

};



class interfaceEvent_c:public QObject
{
    Q_OBJECT
public:
    interfaceEvent_c(const QObject * sender, const char * signal, interfaceEvent_c *pPrevEvent, int eventType, void *param, interface_c *interface, bool repeat);
    ~interfaceEvent_c();
    const QObject *sender;
    const char *signal;
    interfaceEvent_c *pPrevEvent;
    int eventType;
    interface_c *interface;
    bool repeat;
    void *data;
    interfaceEvent_c *pNextEvent;
    int count;
    void destroy(void);
    void updateParameters(void *param);
    private slots:
        void eventProcess(void)    ;
};






#endif // EVENTS_H
