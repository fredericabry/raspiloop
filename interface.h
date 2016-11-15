#ifndef INTERFACE_H
#define INTERFACE_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include "alsa_capture.h"
#include <stdbool.h>


#include <QFile>
#include <QTextStream>

#include <qdebug.h>
#include <qtimer.h>
#include <qelapsedtimer.h>

#include "playback_port_c.h"
#include "playback_loop_c.h"

#include "capture_loop_c.h"
#include "capture_port_c.h"

#include "QKeyEvent"
#include "qdir.h"


class interface_c:public QThread
{
    Q_OBJECT
public:
    interface_c(MainWindow *parent);
    MainWindow *parent;
    void run() Q_DECL_OVERRIDE;


private slots:
    void keyInput(QKeyEvent *e);


};



#endif // INTERFACE_H

