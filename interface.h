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
    void showPlayLoops();
    click_c *pClick;


    void Afficher(QString txt);
    bool isRecording;

private slots:
    void keyInput(QKeyEvent *e);
signals:
    void setTempo(int);


};



#endif // INTERFACE_H

