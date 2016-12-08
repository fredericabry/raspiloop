#include "dialog_newcontrol.h"
#include "ui_dialog_newcontrol.h"
#include "qmainwindow.h"
#include "interface.h"


dialog_newcontrol::dialog_newcontrol(QWidget *parent, QMainWindow *mainwindow,interface_c *pInterface) :
    QDialog(parent),
    mainwindow(mainwindow),
    pInterface(pInterface),
    ui(new Ui::dialog_newcontrol)
{
    ui->setupUi(this);
    connect(this->ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));


}




void dialog_newcontrol::keyPressEvent(QKeyEvent *e)
{
if ((e->key()==Qt::Key_CapsLock)
  ||(e->key()==Qt::Key_NumLock))
        return;

   ui->consoleKey->setText(QString::number(e->key()));

}















dialog_newcontrol::~dialog_newcontrol()
{

    ((QWidget*)parent())->setEnabled(true);

    delete ui;
}
