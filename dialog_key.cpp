#include "dialog_key.h"
#include "ui_dialog_key.h"
#include "qmessagebox.h"
#include "alsa_midi.h"
#include "qdebug.h"
#include "control.h"
dialog_key::dialog_key(QWidget *parent,int type):
    QDialog(parent),
    ui(new Ui::dialog_key),
    type(type)
{


    ui->setupUi(this);
    connect(ui->bCancel,SIGNAL(pressed()),this,SLOT(deleteLater()));

    QRect geo = this->geometry();

    if(type == 0)
    {
        ui->bDone->setVisible(false);
        ui->consoleMidi->setVisible(false);
        ui->label->setText("Press any key");
        this->setGeometry(geo);

    }
    else if (type == 1)
    {
        ui->bDone->setVisible(true);
        connect(ui->bDone,SIGNAL(pressed()),this,SLOT(doneMidi()));
        ui->consoleMidi->setVisible(true);
        ui->label->setText("Send any midi control");

    }


}

dialog_key::~dialog_key()
{
    ((QWidget*)parent())->setEnabled(true);
    delete ui;
}



void dialog_key::getMidiCC(QString msg, int code)
{

    //  qDebug()<<msg<<code;
    midiMsg = msg;
    ui->consoleMidi->setText(msg);
}




void dialog_key::getMidiMsg(QString msg)
{
    midiMsg = msg;


    ui->consoleMidi->setText(midiMsg);
}
void dialog_key::doneMidi()
{

    emit sendMidi(midiMsg);
    deleteLater();

}



void dialog_key::keyPressEvent(QKeyEvent *e)
{
    if ((e->key()==Qt::Key_CapsLock)
            ||(e->key()==Qt::Key_NumLock))
        return;

    emit sendKey(e);
    deleteLater();
}
