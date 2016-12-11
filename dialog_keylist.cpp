#include "dialog_keylist.h"
#include "ui_dialog_keylist.h"
#include "config_file.h"
#include "qdebug.h"
#include "qmessagebox.h"

dialog_keylist::dialog_keylist(QWidget *parent,interface_c *pInterface) :
    QDialog(parent),
    ui(new Ui::dialog_keylist),
    pInterface(pInterface)

{
    ui->setupUi(this);
    connect(ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));



    draw();


}


void dialog_keylist::draw(void)
{



    int x = 10;
    int y = 10;

    QStringList keyList;

    fileGetControlKeyList(&keyList);




    for(int i = 0;i<keyList.size();i++)
    {

        QPushButton *deviceButton = new QPushButton(this);

        connect(deviceButton,SIGNAL(pressed()),this,SLOT(clickButton()));
        QFont font;
        font.setPointSize(6);
        deviceButton->setFont(font);
        deviceButton->setText(keyList[i]);
        QRect pos = deviceButton->geometry();
        pos.setWidth(150);
        pos.setHeight(40);
        pos.moveTo(x,y);
        deviceButton->setGeometry(pos);
        x+=0;
        y+=40;
        if(y>400) {y=10;x+=150;}
    }


}










void dialog_keylist::clickButton(void)
{
    QPushButton *pButton = (QPushButton*)(QObject::sender());
    if(pButton)

    {
        QStringList txtList;
        QString key = pButton->text();
        fileGetControl(key,&txtList);


        QMessageBox msgBox;
        msgBox.setText("Control "+key+"\n\n"+txtList.join("\n"));



        QAbstractButton *removeButton = msgBox.addButton(tr("Remove"), QMessageBox::ActionRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);
        QFont font = msgBox.font();
        font.setPixelSize(15);
        msgBox.setFont(font);
        msgBox.exec();
        if (msgBox.clickedButton() == removeButton)
        {
             fileRemoveControl(key);
             pButton->setVisible(false);

             pInterface->makeActiveControlsList();
        }


    }

}





dialog_keylist::~dialog_keylist()
{
    ((QWidget*)parent())->setEnabled(true);
    delete ui;
}
