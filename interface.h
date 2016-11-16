#ifndef INTERFACE_H
#define INTERFACE_H

#include "mainwindow.h"
#include "qthread.h"
#include "playback_loop_c.h"
#include "QKeyEvent"





class interface_c:public QThread
{
    Q_OBJECT
public:
    interface_c(MainWindow *parent);
    MainWindow *parent;
    void run() Q_DECL_OVERRIDE;

    int getPlayLoopCount();
    playback_loop_c* firstLoop; //first of the playback loop list
    playback_loop_c* findLastPlaybackLoop(void);
    void Afficher(QString txt);
private slots:
    void keyInput(QKeyEvent *e);


};



#endif // INTERFACE_H

