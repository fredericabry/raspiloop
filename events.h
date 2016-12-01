#ifndef EVENTS_H
#define EVENTS_H

#define EVENT_CAPTURE 1
#define EVENT_CREATE_PLAY 2
#define EVENT_PLAY_RESTART 3


#include <QMainWindow>
#include "interface.h"
#include "playback_loop_c.h"
#include "qmutex.h"

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
    std::vector<playback_port_c*>pPlayPorts;

    capture_loop_c **pCaptureLoop;
};




struct playData_s
{
    int id;
    std::vector<playback_port_c*>pPlayPorts;
    long length;
    int skipevent;//number of event call to skip before actually processing the event
    syncoptions syncMode;
    status_t status;
    capture_loop_c *pCaptureLoop;

};

struct restartplayData_s
{
    int id;
    int skipevent;
    status_t status;
    playback_loop_c *pLoop;

};



class interfaceEvent_c:public QObject
{
    Q_OBJECT
public:
    interfaceEvent_c(const QObject * sender, const char * signal,const int eventType, void *param, interface_c *interface, bool repeat);
    ~interfaceEvent_c();
    const QObject *sender;
    const char *signal;
    interfaceEvent_c *pPrevEvent;
    const int eventType;
    interface_c *interface;
    bool repeat;

    interfaceEvent_c *pNextEvent;
    int count;
    void destroy(void);
    void updateParameters(void *param);
    void addToList(void);
    void removeFromList(void);
    void test(void);


    captureData_s *captureData;
    playData_s *playData;
    restartplayData_s *restartplayData;

private slots:
    void eventProcess(void)    ;

};






#endif // EVENTS_H
