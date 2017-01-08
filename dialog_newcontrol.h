#ifndef DIALOG_NEWCONTROL_H
#define DIALOG_NEWCONTROL_H

#include <QDialog>

class interface_c;
class QMainWindow;


namespace Ui {
class dialog_newcontrol;
}

class dialog_newcontrol : public QDialog
{
    Q_OBJECT

public:

    explicit dialog_newcontrol(QWidget *parent = 0,QMainWindow *mainwindow=NULL,interface_c* pInterface = NULL);
    QMainWindow *mainwindow;
    interface_c *pInterface;
    ~dialog_newcontrol();

    void refreshConsole(void);
    void showActive(QStringList *txt);


    void setControlList(const QStringList &value);

    void setKey(const QString value);

    void setMidi(const QString &value);

    void setOldKey(const QString &value);

    void setOldMidi(const QString &value);

private:
    Ui::dialog_newcontrol *ui;
    QStringList controlList;
    QString key;   
    QString midi;
    int activeControl;
    QString oldKey;//contains the former key if the dialog is opened during an editing
    QString oldMidi;//contains the former key if the dialog is opened during an editing
public slots:
    void openControlList(void);
    void getControl(QString txt);
    void removeLastControl();
    void keyChoice(void);
    void getKey(QKeyEvent *e);
    void save(void);
    void selectUp(void);
    void selectDown(void);
    void midiChoice(void);
    void getMidiMsg(QString msg);

};

#endif // DIALOG_NEWCONTROL_H
