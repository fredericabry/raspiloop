#ifndef INTERFACE_H
#define INTERFACE_H

#include "mainwindow.h"
#include "qthread.h"
#include "QKeyEvent"
#include "qmutex.h"
#include "qelapsedtimer.h"
#include "qtimer.h"


class playback_loop_c;
class capture_loop_c;
class capture_port_c;
class playback_port_c;
class interfaceEvent_c;
class click_c;
class alsa_playback_device;
class alsa_capture_device;
class control_c;
class interface_c;
class controlList_c;
class alsa_midi_capture_device;

enum status_t {IDLE,//not playing : paused
                  PLAY,//Normal behavior
                  SILENT,//playing muted
                  HIDDEN};//playing muted and not shown in the list (created by capture loop but not played yet)



enum syncoptions {NOSYNC,//no synchronization, loop are played as they are recorder
                  CLICKSYNC,//synchronization to a pre defined click
                  AUTOCLICK};//click is defined by the first recorded loop


enum mixStrategies {AUTO,PRESET};




enum controlType_e {VOID,INT,QSTRING};



struct controlId
{
    controlType_e type;
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
    bool isIdFree(int id);//check if id is currentyle used to play or record
    playback_loop_c* findPlayLoopById(int id);
    capture_loop_c* findCaptureLoopById(int id);
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
    void printPlaybackLoopList(void);
    void printCaptureLoopList(void);

    interfaceEvent_c *firstEvent;//first event registered, begining of the events list
    interfaceEvent_c* findLastEvent(void);//return a pointer to the last event registered
    void removeRestartEvents(int id);//remove all events with id and envetType
    bool isEventValid(interfaceEvent_c* pEvent);
    int getEventsCount();
    void removeAllEvents();
    void alsaDestroy();

    void moveClick(std::vector<playback_port_c*> pPorts);

    capture_port_c* findCapturePortByChannel(int channel);
    playback_port_c* findPlaybackPortByChannel(int channel);

    void updatePortsConsole(void);

    bool isMixOver(void);//test if mix is done in every playback port for sync

    void showPlayLoops();
    click_c *pClick;

    QString statusToString(status_t status);

    void Afficher(QString txt);
    bool isRecording;

    syncoptions synchroMode;//specifies the click synchronisation mode;
    QMutex captureListMutex;
    QMutex playbackListMutex;
    QMutex eventListMutex;


    QTimer *timerMix;





    std::vector<alsa_capture_device*> captureDevicesList;
    std::vector<alsa_playback_device*> playbackDevicesList;
    std::vector<alsa_midi_capture_device*> midiCaptureDevicesList;
    //std::vector<alsa_midi_playback_device*> playbackDevicesList;




    std::vector<playback_port_c*> playbackPortsList;
    std::vector<capture_port_c*> capturePortsList;
    std::vector<playback_port_c*> selectedPlaybackPortsList;
    std::vector<capture_port_c*> selectedCapturePortsList;


    unsigned int playbackPortsCount,capturePortsCount;

    mixStrategies getMixStrategy() const;
    void setMixStrategy(mixStrategies value);

    unsigned int getMixLoopNumber();
    void setMixLoopNumber(unsigned int value);


    playback_loop_c *selectedPlayLoop;//selected play loop highlighted in the interface



 //functions defined in control.cpp
    void createCapture(int desiredId);//create capture loop
    void createCaptureAndPlay(int desiredId);//create capture loop and autoplay loop when capture is stoped
    void stopCapture(int Id);
    void selectPlayback(int channel);
    void unselectPlayback(int channel);

    void selectCapture(int channel);
    void selectNextCapture(void);

    void startstopLoop(int id);
    void startLoop(int id);
    void stopLoop(int id);
    void stopAllLoops(void);
    void muteunmuteLoop(int id);
    void muteunmuteAllLoops(void);
    void unselectAllPlaybacks(void);
    void selectAllPlaybacks(void);
    void stopSelectedLoop(void);

    void waitForNextKey(void);
    void startstopClick(void);

    void moveLoop(int);
    void moveSelectedLoop();

    void selectNextLoop(void);
    void selectPrevLoop(void);




    void createControls(void);
    std::vector<controlId*> controlList; //available control options
    std::vector<controlList_c*> controlLists;//registered controls

    void registerControl(QString key,void (interface_c::*function)());
    void registerControl(QString key,void (interface_c::*function)(QString));
    void registerControl(QString key,void (interface_c::*function)(int));
    void makeActiveControlsList(void);
    controlId *identifyControl(QString control);
    int getControlParamInt(QString control);
    QString getControlParamQString(QString control);
    bool activateControl(QString key);
    bool activateMidi(QString midiMsg);




    bool getAllMute() const;
    void setAllMute(bool value);

private:
    //mix strategy:
    mixStrategies mixStrategy;
    unsigned int mixLoopNumber;
    bool allMute;//true if all playloops are "hard" muted
    QStringList midiBuffer;
    unsigned int midiIndex;




public slots:
    void clickPlayStop(void);
    void checkMidiMsg(QString);

private slots:
    void keyInput(QKeyEvent *e);
    void createInterfaceEvent(const QObject * sender, const char * signal, int eventType, void *param, bool repeat, interfaceEvent_c **pEvent);

signals:
    void setTempo(int);
    void playbackLoopList(QString);
    void captureLoopList(QString);
    void sendPlaybackConsole(QString txt);
    void sendCaptureConsole(QString txt);
    void getMidiMsg(QString msg);



};



#endif // INTERFACE_H

