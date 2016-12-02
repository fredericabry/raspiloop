#include "dialogdevice.h"
#include "ui_dialogdevice.h"
#include "interface.h"
#include "alsa_util.h"
#include "qdebug.h"

DialogDevice::DialogDevice(QWidget *parent, interface_c *pInterface,int type):
    QDialog(parent),
    ui(new Ui::DialogDevice),
    pInterface(pInterface)
{
    ui->setupUi(this);


qDebug()<<"device"<<type;
    connect(this->ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));

    QStringList cardNames,cardLongNames;


    int l;


    if(type == 0) //playback devices
    {

        this->ui->labelTitle->setText("Available playback devices");
        l = getCardListLength(SND_PCM_STREAM_PLAYBACK);
        getCardList(SND_PCM_STREAM_PLAYBACK,&cardNames,&cardLongNames);
        if(l>6) {qDebug()<<"bug: too many devices";l = 6;}


    }

    else
    {

        this->ui->labelTitle->setText("Available capture devices");
        l = getCardListLength(SND_PCM_STREAM_CAPTURE);
        getCardList(SND_PCM_STREAM_CAPTURE,&cardNames,&cardLongNames);
        if(l>6) {qDebug()<<"bug: too many devices";l = 6;}

    }



    for (int i = 0;i<l;i++)
    {
        QPushButton *deviceButton = new QPushButton(this);
        QFont font;
        font.setPointSize(6);
        deviceButton->setFont(font);
        deviceButton->setText(cardLongNames[i]);
        QRect pos = deviceButton->geometry();
        pos.setWidth(600);
        pos.setHeight(40);
        pos.moveTo(40,40+60*i);
        deviceButton->setGeometry(pos);
      //  qDebug()<<cardNames[i];
    }










}

DialogDevice::~DialogDevice()
{
    delete ui;
}
