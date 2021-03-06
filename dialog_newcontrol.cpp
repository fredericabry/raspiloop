#include "dialog_newcontrol.h"
#include "ui_dialog_newcontrol.h"
#include "qmainwindow.h"
#include "interface.h"
#include "qdebug.h"
#include "dialog_controllist.h"
#include "dialog_key.h"
#include "qmessagebox.h"
#include "config_file.h"
#include "control.h"

dialog_newcontrol::dialog_newcontrol(QWidget *parent, QMainWindow *mainwindow,interface_c *pInterface) :
    QDialog(parent),
    mainwindow(mainwindow),
    pInterface(pInterface),
    ui(new Ui::dialog_newcontrol)
{

    ui->setupUi(this);
    oldKey="";
    oldMidi="";
    connect(this->ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));
    connect(ui->bAddControl,SIGNAL(pressed()),this,SLOT(openControlList()));
    connect(ui->bRemoveControl,SIGNAL(pressed()),this,SLOT(removeLastControl()));
    connect(ui->bKeyChoice,SIGNAL(pressed()),this,SLOT(keyChoice()));
    connect(ui->bSave,SIGNAL(pressed()),this,SLOT(save()));
    connect(ui->bDown,SIGNAL(pressed()),this,SLOT(selectDown()));
    connect(ui->bUp,SIGNAL(pressed()),this,SLOT(selectUp()));
    connect(ui->bMidiChoice,SIGNAL(pressed()),this,SLOT(midiChoice()));


    key.clear();
}




void dialog_newcontrol::getKey(QKeyEvent *e)
{


    setKey((QString)(QKeySequence(e->key()).toString()));

}

void dialog_newcontrol::getMidiMsg(QString msg)
{

    setMidi(msg);

}



void dialog_newcontrol::midiChoice()
{
    this->setEnabled(false);
    dialog_key *dialog = new dialog_key(this,1);
    connect(dialog,SIGNAL(sendMidi(QString)),this,SLOT(getMidiMsg(QString)));
    connect(pInterface,SIGNAL(getMidiMsg(QString)),dialog,SLOT(getMidiMsg(QString)));
    connect(pInterface,SIGNAL(getMidiCC(QString,int)),dialog,SLOT(getMidiCC(QString,int)));
    dialog->show();
    QPoint pos = this->pos();
    pos.setX(300);
    pos.setY(100);
    dialog->move(pos);
}


void dialog_newcontrol::openControlList(void)
{
    this->setEnabled(false);
    dialog_controllist *dialog = new dialog_controllist(this,((QMainWindow*)parent()),pInterface);
    connect(dialog,SIGNAL(sendText(QString)),this,SLOT(getControl(QString)));
    dialog->show();
    QPoint pos = this->pos();
    pos.setX(0);
    pos.setY(0);
    dialog->move(pos);
}



void dialog_newcontrol::showActive(QStringList *txt)
{
    if(activeControl > controlList.size() -1) activeControl = controlList.size() -1;
    if(activeControl<0) activeControl = 0;

    if(controlList.size()<= 0) return;

    *txt = controlList;

    QString buf = "<font color=\"#ff4040\">"+controlList.at(activeControl)+"</font>";

    txt->replace(activeControl,buf);
}

void dialog_newcontrol::setControlList(const QStringList &value)
{
    controlList = value;


}

void dialog_newcontrol::setKey(const QString value)
{
    key = value;

    ui->consoleKey->setText(key);

    QTextCursor cursor = ui->consoleKey->textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    textBlockFormat.setAlignment(Qt::AlignCenter);//or another alignment
    cursor.mergeBlockFormat(textBlockFormat);
    ui->consoleKey->setTextCursor(cursor);
}

void dialog_newcontrol::setMidi(const QString &value)
{
    midi = value;
}

void dialog_newcontrol::setOldKey(const QString &value)
{
    oldKey = value;
}

void dialog_newcontrol::setOldMidi(const QString &value)
{
    oldMidi = value;
}


void dialog_newcontrol::getControl(QString txt)
{
    //controlList.append(txt);
    controlList.insert(activeControl+1,txt);
    activeControl ++;
    refreshConsole();
}

void dialog_newcontrol::removeLastControl()
{
    if(controlList.size()<=0) return;


    controlList.removeAt(activeControl);
    activeControl --;
    if(activeControl<0) activeControl = 0;
    refreshConsole();



}

void dialog_newcontrol::selectDown(void)
{
    if(controlList.size()<=0) return;

    activeControl ++;
    if(activeControl>controlList.size()-1) activeControl = 0;
    refreshConsole();
}

void dialog_newcontrol::selectUp(void)
{
    if(controlList.size()<=0) return;

    activeControl --;
    if(activeControl<0) activeControl = controlList.size()-1;
    refreshConsole();
}


void dialog_newcontrol::save(void)
{
    if((key.size()==0)&&(midi.size() == 0))
    {
        QMessageBox msgBox;
        msgBox.setText("Error: no key or midi control set");
        QFont font = msgBox.font();
        font.setPixelSize(20);
        msgBox.setFont(font);
        msgBox.exec();
        return;
    }
    if(controlList.size()==0)
    {
        QMessageBox msgBox;
        msgBox.setText("Error: control list empty");
        QFont font = msgBox.font();
        font.setPixelSize(20);
        msgBox.setFont(font);
        msgBox.exec();
        return;
    }

    if(key.size()>0)
    {

        QString keyword = "KEY_"+key;

        if(fileIsControlDefined(keyword))
        {
            QMessageBox msgBox;
            msgBox.setText("Control already defined");
            QAbstractButton *removeButton = msgBox.addButton(tr("Replace"), QMessageBox::ActionRole);
            msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);
            QFont font = msgBox.font();
            font.setPixelSize(20);
            msgBox.setFont(font);
            msgBox.exec();
            if (msgBox.clickedButton() != removeButton)
            {
                return;
            }
        }


        fileSetControl(keyword,controlList);

    }
    if(midi.size()>0)
    {

        QString keyword = "MIDI_"+midi;

        if(fileIsControlDefined(keyword))
        {
            QMessageBox msgBox;
            msgBox.setText("Control already defined");
            QAbstractButton *removeButton = msgBox.addButton(tr("Replace"), QMessageBox::ActionRole);
            msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);
            QFont font = msgBox.font();
            font.setPixelSize(20);
            msgBox.setFont(font);
            msgBox.exec();
            if (msgBox.clickedButton() != removeButton)
            {
                return;
            }
        }


        fileSetControl(keyword,controlList);

    }

    if((oldMidi!="")&&(midi!=oldMidi))
        fileRemoveControl("MIDI_"+oldMidi);
    if((oldKey!="")&&(key!=oldKey))
        fileRemoveControl("KEY_"+oldKey);

    pInterface->makeActiveControlsList();



    deleteLater();

}





void dialog_newcontrol::refreshConsole(void)
{
    QStringList show;
    showActive(&show);

    QString cmdTxt = show.join("<br>");
    cmdTxt.replace(CONTROL_MARKER," ");
    ui->commandList->setHtml(cmdTxt);


}



void dialog_newcontrol::keyChoice(void)
{

    this->setEnabled(false);
    dialog_key *dialog = new dialog_key(this,0);
    connect(dialog,SIGNAL(sendKey(QKeyEvent*)),this,SLOT(getKey(QKeyEvent*)));
    dialog->show();
    QPoint pos = this->pos();
    pos.setX(300);
    pos.setY(160);
    dialog->move(pos);

}






dialog_newcontrol::~dialog_newcontrol()
{

    ((QWidget*)parent())->setEnabled(true);

    delete ui;
}
