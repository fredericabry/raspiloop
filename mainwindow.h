#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>





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



    void setClickText(int tempo);
    void setClickButton(bool status);

    QTimer *clickTimer;
    QTimer *stepTimer;
    QTimer *tester;


    void AfficherListe(QStringList);
    void connectToCard(void);
    ~MainWindow();

    void keyPressEvent(QKeyEvent *e);

private:



    Ui::MainWindow *ui;

private slots:
    void cardchoicerefresh(void);
    void chooseCard(void);
    void setLoopList(QString txt);


    void shutdown(void);




signals:
    void sendKey(QKeyEvent *e);
    void clickUp(void);
    void clickDown(void);

};

#endif // MAINWINDOW_H
