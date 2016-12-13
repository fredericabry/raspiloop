#include "dialog_device.h"
#include "ui_dialog_device.h"
#include "interface.h"
#include "alsa_util.h"
#include "qdebug.h"
#include "dialog_parameters.h"
#include "config_file.h"
#include "alsa_midi.h"

dialog_device::dialog_device(QWidget *parent,QMainWindow* mainwindow, interface_c *pInterface,int type):
    QDialog(parent),
    mainwindow(mainwindow),
    ui(new Ui::dialog_device),
    type(type),
    pInterface(pInterface)
{
    ui->setupUi(this);

    //  signalMapper = new QSignalMapper(this);


    connect(this->ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));
    connect(this->ui->bRefresh,SIGNAL(pressed()),this,SLOT(refresh()));



    draw();

    //  connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(clicked(QString)));




}




void dialog_device::draw(void)
{

    QStringList cardNamesFile;

    devicesCount = 0;


    if(type == 0) //playback devices
    {

        this->ui->labelTitle->setText("Available playback devices");
        devicesCount = getCardListLength(SND_PCM_STREAM_PLAYBACK);
        getCardList(SND_PCM_STREAM_PLAYBACK,&cardNames,&cardLongNames);
        if(devicesCount>6) {qDebug()<<"bug: too many devices";devicesCount = 6;}
        keyWord = KEYWORD_PLAYBACK_LIST;


    }

    else if(type == 1)
    {

        this->ui->labelTitle->setText("Available capture devices");
        devicesCount = getCardListLength(SND_PCM_STREAM_CAPTURE);
        getCardList(SND_PCM_STREAM_CAPTURE,&cardNames,&cardLongNames);
        if(devicesCount>6) {qDebug()<<"bug: too many devices";devicesCount = 6;}
        keyWord = KEYWORD_CAPTURE_LIST;

    }
    else if(type == 2)
    {

        this->ui->labelTitle->setText("Available MIDI capture devices");

        alsa_findMidiDevices(SND_RAWMIDI_STREAM_INPUT,&cardLongNames,&cardNames);
        devicesCount = cardLongNames.size();
        if(devicesCount>6) {qDebug()<<"bug: too many devices";devicesCount = 6;}
        keyWord = KEYWORD_MIDI_CAPTURE_LIST;

    }



    extractParameter(keyWord, &cardNamesFile);

    for (unsigned int i = 0;i<devicesCount;i++)
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
        deviceButton->setCheckable(true);

        if(cardNamesFile.contains(cardNames[i])) //device is in config file already
        {
            deviceButton->setChecked(true);
        }


        deviceButton->show();
        buttonsList.push_back(deviceButton);

    }

}





void dialog_device::refresh()
{
    for(auto pButton:buttonsList)
        pButton->deleteLater();

    buttonsList.clear();

    draw();

}










dialog_device::~dialog_device()
{
    ((QWidget*)parent())->setEnabled(true);


    QStringList nuParams;

    for (unsigned int i = 0;i<devicesCount;i++)
    {
        if(buttonsList.size()>=i)
            if(buttonsList[i]->isChecked())
                nuParams.append(cardNames[i]);


    }
    if(cardNames.size()>0)
        setParameter(keyWord,nuParams,true);

    delete ui;
}


