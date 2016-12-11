#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>
#include <qtimer.h>
#include <qelapsedtimer.h>


#include "alsa_util.h"

#include "QKeyEvent"
#include "interface.h"

#include "dialog_device.h"
#include "dialog_parameters.h"
#include "dialog_newcontrol.h"
#include "dialog_controllist.h"
#include "dialog_keylist.h"
#define LOOP_LENGTH 2000









interface_c *mainInterface;






void MainWindow::setClickText(int tempo)
{
    ui->lcdClick->display(tempo);
}

void MainWindow::setPlaybackConsole(QString txt)
{
    ui->consolePlayback->setText(txt);
}

void MainWindow::setCaptureConsole(QString txt)
{
    ui->consoleCapture->setText(txt);
}




void MainWindow::setClickButton(bool status)
{
    if(status) ui->bClick->setText("Stop");
    else ui->bClick->setText("Start");


}



void MainWindow::shutdown()
{

    mainInterface->destroy();
    exit(0);

}


void MainWindow::setLoopList(QString txt)
{
    ui->loopList->setText(txt);

}



void MainWindow::dialogInputDevice(void)
{
    this->setEnabled(false);
    DialogDevice *dialog = new DialogDevice(this,this,mainInterface,1);
    dialog->show();
    QPoint pos = this->pos();
    pos.setX(20);
    pos.setY(20);
    dialog->move(pos);
}
void MainWindow::dialogOutputDevice(void)
{


}


void MainWindow::dialogConfig(void)
{
    this->setEnabled(false);
    dialog_parameters *dialog = new dialog_parameters(this,mainInterface);
    dialog->show();
    QPoint pos = this->pos();
    pos.setX(20);
    pos.setY(20);
    dialog->move(pos);

}



void MainWindow::keyPressEvent(QKeyEvent *e)
{
    emit sendKey(e);//forward the key to the interface thread

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{




    ui->setupUi(this);



    connect(ui->bClickUp,SIGNAL(pressed()),this,SIGNAL(clickUp()));
    connect(ui->bClickDown,SIGNAL(pressed()),this,SIGNAL(clickDown()));

    connect(ui->bstop,SIGNAL(pressed()),this,SLOT(shutdown()));
    connect(ui->bConfig,SIGNAL(pressed()),this,SLOT(dialogConfig()));

    ui->bClick->setCheckable(false);




    hw_info data;
    data = get_device_info("hw:1,1",SND_PCM_STREAM_PLAYBACK);


    /*
    Afficher(data.name);
    Afficher("max playback "+n2s(data.max_playback));
    Afficher("min playback "+n2s(data.min_playback));
    Afficher("max capture "+n2s(data.max_capture));
    Afficher("min capture "+n2s(data.min_capture));
    Afficher("max rate "+n2s(data.max_rate) + "Hz");
    Afficher("min rate "+n2s(data.min_rate) + "Hz");
    Afficher("max buffer" + n2s(data.max_buffer_size));
    Afficher("min buffer" + n2s(data.min_buffer_size));
    pLeft = alsa_playback_port_by_num(0);
    pLeft = alsa_playback_port_by_num(1);
*/




    // QString txt = "taskset -cp 3 "+n2s(QCoreApplication::applicationPid());
    QString  txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());
    txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());
    txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());
    txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());





    mainInterface = new interface_c(this);
    mainInterface->start();

    connect(ui->bClick,SIGNAL(pressed()),mainInterface,SLOT(clickPlayStop()));


    connect(this,SIGNAL(sendKey(QKeyEvent*)),mainInterface,SLOT(keyInput(QKeyEvent*)));







}





MainWindow::~MainWindow()
{


    delete ui;


}
