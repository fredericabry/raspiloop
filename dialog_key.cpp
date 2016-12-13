#include "dialog_key.h"
#include "ui_dialog_key.h"
#include "qmessagebox.h"
#include "alsa_midi.h"
#include "qdebug.h"

dialog_key::dialog_key(QWidget *parent,int type):
    QDialog(parent),
    ui(new Ui::dialog_key),
    type(type)
{


    ui->setupUi(this);

    QRect geo = this->geometry();

    if(type == 0)
    {
        ui->bDone->setVisible(false);
        ui->consoleMidi->setVisible(false);
        ui->label->setText("Press any key");
        geo.setHeight(150);
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


void dialog_key::getMidiMsg(QString msg)
{
    midiMsg.append(msg);

    if(midiMsg.size()>MIDI_MAX_LENGTH)
    {
        QMessageBox msgBox;
        msgBox.setText("Too many controls in midi sequence (max: "+QString::number(MIDI_MAX_LENGTH)+"7)");
        msgBox.addButton(tr("Ok"), QMessageBox::ActionRole);
        QFont font = msgBox.font();
        font.setPixelSize(20);
        msgBox.setFont(font);
        msgBox.exec();
        midiMsg.clear();
    }


    ui->consoleMidi->setText(midiMsg.join("\n"));
}
void dialog_key::doneMidi()
{

    emit sendMidi(midiMsg.join(";"));
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
