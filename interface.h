#ifndef INTERFACE_H
#define INTERFACE_H

#include "mainwindow.h"
#include "qthread.h"
#include "QKeyEvent"



class playback_loop_c;
class capture_loop_c;
class capture_port_c;
class playback_port_c;
class interfaceEvent_c;
class click_c;

enum status_t {IDLE,//not playing : paused
                  PLAY,//Normal behavior
                  SILENT};//playing muted



enum syncoptions {NOSYNC,//no synchronization, loop are played as they are recorder
                  CLICKSYNC,//synchronization to a pre defined click
                  AUTOCLICK};//click is defined by the first recorded loop



class interface_c:public QThread
{
    Q_OBJECT
public:
    interface_c(MainWindow *parent);
    MainWindow *parent;
    void run() Q_DECL_OVERRIDE;

    int generateNewId(void);//return an available id neither used by a capture nor a playback loop
    playback_loop_c* findPlayLoopById(int id);
    void init(void);

    playback_loop_c* firstPlayLoop; //first of the playback loops list
    playback_loop_c* findLastPlaybackLoop(void);
    int getPlayLoopsCount();
    void removeAllPlaybackLoops(void);

    capture_loop_c* firstCaptureLoop;//first of the capture loops list
    capture_loop_c* findLastCaptureLoop(void);
    int getCaptureLoopsCount();

    interfaceEvent_c *firstEvent;//first event registered, begining of the events list
    interfaceEvent_c* findLastEvent(void);//return a pointer to the last event registered
    bool isEventValid(interfaceEvent_c* pEvent);
    int getEventsCount();



    void startRecord(playback_port_c *pPlayPort, capture_port_c *pCapturePort);

    void showPlayLoops();
    click_c *pClick;


    void Afficher(QString txt);
    bool isRecording;

    syncoptions synchroMode;//specifies the click synchronisation mode;

private slots:
    void keyInput(QKeyEvent *e);
    void createInterfaceEvent(const QObject * sender, const char * signal, int eventType, void *param, bool repeat, playback_loop_c *pLoop);

signals:
    void setTempo(int);



};



#endif // INTERFACE_H

