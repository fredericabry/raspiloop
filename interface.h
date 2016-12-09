#ifndef INTERFACE_H
#define INTERFACE_H

#include "mainwindow.h"
#include "qthread.h"
#include "QKeyEvent"
#include "qmutex.h"
#include "qelapsedtimer.h"


class playback_loop_c;
class capture_loop_c;
class capture_port_c;
class playback_port_c;
class interfaceEvent_c;
class click_c;
class alsa_playback_device;
class alsa_capture_device;
class control;
class interface_c;

enum status_t {IDLE,//not playing : paused
                  PLAY,//Normal behavior
                  SILENT};//playing muted



enum syncoptions {NOSYNC,//no synchronization, loop are played as they are recorder
                  CLICKSYNC,//synchronization to a pre defined click
                  AUTOCLICK};//click is defined by the first recorded loop


enum mixStrategies {AUTO,PRESET};




enum type_e {VOID,INT,QSTRING};



struct controlId
{
    type_e type;
    QString key;
    void (interface_c::*exec_void)();
    void (interface_c::*exec_qstring)(QString);
    void (interface_c::*exec_int)(int);
};





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
    bool isCaptureLoopValid(capture_loop_c * pCapture);//is capture loop in the list ?
    int getCaptureStatus(capture_loop_c **pCaptureLoop);//retur the stauts of a record loop pointer

    bool clickStatus;
    void printLoopList(void);

    interfaceEvent_c *firstEvent;//first event registered, begining of the events list
    interfaceEvent_c* findLastEvent(void);//return a pointer to the last event registered
    bool isEventValid(interfaceEvent_c* pEvent);
    int getEventsCount();
    void removeAllEvents();

    void destroy();
    void startRecord(std::vector<playback_port_c*>pPlayPorts, capture_port_c *pCapturePort, capture_loop_c **pCaptureLoop, long length);
    void moveClick(std::vector<playback_port_c*> pPorts);

    void createControls(void);
    std::vector<controlId*> controlList;


    void registerControl(QString key,void (interface_c::*function)());
    void registerControl(QString key,void (interface_c::*function)(QString));
    void registerControl(QString key,void (interface_c::*function)(int));







    void showPlayLoops();
    click_c *pClick;

    QString statusToString(status_t status);

    void Afficher(QString txt);
    bool isRecording;

    syncoptions synchroMode;//specifies the click synchronisation mode;
    QMutex captureListMutex;
    QMutex playbackListMutex;
    QMutex eventListMutex;


    QElapsedTimer telapsed;


    std::vector<alsa_capture_device*> captureDevicesList;
    std::vector<alsa_playback_device*> playbackDevicesList;
    std::vector<playback_port_c*> playbackPortsList;
    std::vector<capture_port_c*> capturePortsList;
    unsigned int playbackPortsCount,capturePortsCount;

    mixStrategies getMixStrategy() const;
    void setMixStrategy(mixStrategies value);

    unsigned int getMixLoopNumber();
    void setMixLoopNumber(unsigned int value);








 //functions defined in control.cpp
    void createCapture(int desiredId);
    void stopCapture(int Id);
    void selectPlayback(int channel);
    void unselectPlayback(int channel);
    void selectCapture(int channel);
    void unselectCapture(int channel);
    void startstopLoop(int id);
    void startLoop(int id);
    void stopLoop(int id);
    void stopAllLoops(void);
    void muteunmuteLoop(int id);
    void muteunmuteAllLoops(void);
    void unselectAllPlaybacks(void);
    void selectAllPlaybacks(void);
    void unselectAllCaptures(void);
    void selectAllCaptures(void);
    void waitForNextControl(void);



private:
    //mix strategy:
    mixStrategies mixStrategy;
    unsigned int mixLoopNumber;






public slots:
    void clickPlayStop(void);

private slots:
    void keyInput(QKeyEvent *e);
    void createInterfaceEvent(const QObject * sender, const char * signal, int eventType, void *param, bool repeat, interfaceEvent_c **pEvent);

signals:
    void setTempo(int);
    void loopList(QString);



};



#endif // INTERFACE_H

