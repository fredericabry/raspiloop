#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <alsa/asoundlib.h>



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

    QTimer *clickTimer;

    void Afficher(QString);
    void AfficherI(int i);
    void br();
    void AfficherListe(QStringList);
    void connectToCard(void);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void cardchoicerefresh(void);
    void chooseCard(void);
    void consoleclear(void);
    void link(void);
    void ding(void);
    void topClick(void);
    void updateTempo(int);
signals:

};

#endif // MAINWINDOW_H
