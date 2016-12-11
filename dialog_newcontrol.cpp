#include "dialog_newcontrol.h"
#include "ui_dialog_newcontrol.h"
#include "qmainwindow.h"
#include "interface.h"
#include "qdebug.h"
#include "dialog_controllist.h"
#include "dialog_key.h"
#include "qmessagebox.h"
#include "config_file.h"

dialog_newcontrol::dialog_newcontrol(QWidget *parent, QMainWindow *mainwindow,interface_c *pInterface) :
    QDialog(parent),
    mainwindow(mainwindow),
    pInterface(pInterface),
    ui(new Ui::dialog_newcontrol)
{
    ui->setupUi(this);

    connect(this->ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));
    connect(ui->bAddControl,SIGNAL(pressed()),this,SLOT(openControlList()));
    connect(ui->bRemoveControl,SIGNAL(pressed()),this,SLOT(removeLastControl()));
    connect(ui->bKeyChoice,SIGNAL(pressed()),this,SLOT(keyChoice()));
    connect(ui->bSave,SIGNAL(pressed()),this,SLOT(save()));

    key.clear();
}




void dialog_newcontrol::getKey(QKeyEvent *e)
{
    key = QString::number(e->key());
    ui->consoleKey->setText(key);

    QTextCursor cursor = ui->consoleKey->textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    textBlockFormat.setAlignment(Qt::AlignCenter);//or another alignment
    cursor.mergeBlockFormat(textBlockFormat);
    ui->consoleKey->setTextCursor(cursor);

}


void dialog_newcontrol::openControlList(void)
{

    this->setEnabled(false);
    dialog_controllist *dialog = new dialog_controllist(this,((QMainWindow*)parent()),pInterface);
    connect(dialog,SIGNAL(sendText(QString)),this,SLOT(getControl(QString)));
    dialog->show();
    QPoint pos = this->pos();
    pos.setX(20);
    pos.setY(20);
    dialog->move(pos);

}


void dialog_newcontrol::getControl(QString txt)
{
    controlList.append(txt);
    refreshConsole();
}

void dialog_newcontrol::removeLastControl()
{
    if(controlList.size()>0)
    {
        controlList.removeLast();
        refreshConsole();

    }

}

void dialog_newcontrol::save(void)
{
    if(key.size()==0)
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

    QString keyword = "KEY_"+key;

    if(fileIsControlDefined(keyword))
    {
        QMessageBox msgBox;
        msgBox.setText("Control already defined");
        QAbstractButton *removeButton = msgBox.addButton(tr("Remove"), QMessageBox::ActionRole);
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

    pInterface->makeActiveControlsList();

    deleteLater();

}





void dialog_newcontrol::refreshConsole(void)
{

    ui->commandList->setText(controlList.join("\n"));


}



void dialog_newcontrol::keyChoice(void)
{

    this->setEnabled(false);
    dialog_key *dialog = new dialog_key(this);
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
