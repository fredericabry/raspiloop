#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <alsa/asoundlib.h>
#include <qfile.h>

#include "alsa_util.h"
#include "alsa_playback.h"
#include "alsa_capture.h"
#include "playback_port_c.h"
#include "playback_loop_c.h"
#include "capture_loop_c.h"
#include "capture_port_c.h"



//#define DIRECTORY "/home/pi/usb/"
//#define DIRECTORY "/home/pi/usb1/"
#define DIRECTORY ""

#define RATE 48000

#define n2s QString::number

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QStringList CardList;
    QStringList CardDescription;
    const char **portsIn;
    const char **portsOut;

    QStringList InputCardNames;
    QStringList InputCardLongNames;
    QStringList OutputCardNames;
    QStringList OutputCardLongNames;
    QString InputCardName;
    QString OutputCardName;
    QFile fdebug;

    QTimer *clickTimer;
    QTimer *stepTimer;


    void Afficher(QString);
    void AfficherI(int i);
    void br();
    void AfficherListe(QStringList);
    void connectToCard(void);
    ~MainWindow();

    void keyPressEvent(QKeyEvent *e);





private:
    Ui::MainWindow *ui;

private slots:
    void cardchoicerefresh(void);
    void chooseCard(void);
    void consoleclear(void);
    void play(void);
    void topClick(void);
    void updateTempo(int);
    void record(void);
    void topStep(void);
    void shutdown(void);

signals:
    void sendKey(QKeyEvent *e);

};

#endif // MAINWINDOW_H
