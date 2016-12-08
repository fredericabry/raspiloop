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
    void keyPressEvent(QKeyEvent *e);
private:
    Ui::dialog_newcontrol *ui;
};

#endif // DIALOG_NEWCONTROL_H
