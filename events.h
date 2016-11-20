#ifndef EVENTS_H
#define EVENTS_H

#define EVENT_CAPTURE 1
#define EVENT_PLAY 2


#include <QMainWindow>

class playback_loop_c;
class capture_loop_c;
class capture_port_c;
class playback_port_c;
class interface_c;


struct captureData
{
    capture_port_c *pPort;
    long length;
    bool createPlayLoop;
    playback_port_c *pPlayPort;
};




struct playData
{
    int id;
    playback_port_c *pPlayPort;
    long length;
    bool autoplay;


};


class interfaceEvent_c:public QObject
{
    Q_OBJECT
public:
    interfaceEvent_c(const QObject * sender,const char * signal,interfaceEvent_c *pPrevEvent, int eventType, void *param, interface_c *interface);
    const QObject *sender;
    const char *signal;
    interfaceEvent_c *pPrevEvent;
    int eventType;
    interface_c *interface;
    void *data;
    interfaceEvent_c *pNextEvent;

private slots:
    void eventProcess(void)    ;
};






#endif // EVENTS_H
