#include "dialog_controllist.h"
#include "ui_dialog_controllist.h"
#include "mainwindow.h"
#include "interface.h"
#include "QPushButton"
#include "qobject.h"
#include "qdebug.h"
#include "dialog_numeric.h"


dialog_controllist::dialog_controllist(QWidget *parent,QMainWindow* mainwindow, interface_c *pInterface):
    QDialog(parent),
    mainwindow(mainwindow),
    ui(new Ui::dialog_controllist),
    pInterface(pInterface)
{
    ui->setupUi(this);


    int x = 10;
    int y = 10;

    for(unsigned int i = 0;i<pInterface->controlList.size();i++)
    {
        QPushButton *deviceButton = new QPushButton(this);


        if(pInterface->controlList[i]->type == VOID)
            connect(deviceButton,SIGNAL(pressed()),this,SLOT(pushButton()));
        else if (pInterface->controlList[i]->type == INT)
            connect(deviceButton,SIGNAL(pressed()),this,SLOT(pushButtonInt()));

        QFont font;
        font.setPointSize(6);
        deviceButton->setFont(font);
        deviceButton->setText(pInterface->controlList[i]->key);
        QRect pos = deviceButton->geometry();
        pos.setWidth(250);
        pos.setHeight(40);
        pos.moveTo(x,y);
        deviceButton->setGeometry(pos);
        x+=0;
        y+=40;
        if(y>400) {y=10;x+=250;}

    }


}


void dialog_controllist::openNumeric(void)
{
    this->setEnabled(false);
    dialog_numeric *dialog = new dialog_numeric(this);
    connect(dialog,SIGNAL(exportData(int)),this,SLOT(getNumericData(int)));
    dialog->show();
    QPoint pos = this->pos();
    pos.setX(270);
    pos.setY(50);
    dialog->move(pos);
}


void dialog_controllist::getNumericData(int data)
{
    output+=" "+QString::number(data);
    emit sendText(output);
    deleteLater();
}




void dialog_controllist::pushButtonInt(void)
{


    QPushButton *pButton = (QPushButton*)(QObject::sender());
    if(pButton)
        output = pButton->text();

    openNumeric();

}






void dialog_controllist::pushButton(void)
{

    QPushButton *pButton = (QPushButton*)(QObject::sender());
    if(pButton)

    {
        emit sendText(pButton->text());
        deleteLater();
    }

}



dialog_controllist::~dialog_controllist()
{
    ((QWidget*)parent())->setEnabled(true);
    delete ui;
}
