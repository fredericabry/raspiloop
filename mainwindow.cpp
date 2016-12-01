#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>
#include <qtimer.h>
#include <qelapsedtimer.h>

#include "alsa_capture.h"
#include "alsa_playback.h"
#include "alsa_util.h"

#include "QKeyEvent"
#include "interface.h"



#define LOOP_LENGTH 2000









interface_c *mainInterface;




void MainWindow::chooseCard(void)
{

    //InputCardName = InputCardNames.at(numin);
    //OutputCardName = OutputCardNames.at(numout);
}

void MainWindow::cardchoicerefresh(void)
{
    getCardList(SND_PCM_STREAM_PLAYBACK,&OutputCardNames,&OutputCardLongNames);
    getCardList(SND_PCM_STREAM_CAPTURE,&InputCardNames,&InputCardLongNames);



}










void MainWindow::setClickText(int tempo)
{
    ui->lcdClick->display(tempo);


}


void MainWindow::setClickButton(bool status)
{
    if(status) ui->bClick->setText("Stop");
    else ui->bClick->setText("Start");


}







void MainWindow::shutdown()
{

    exit(0);

}


void MainWindow::setLoopList(QString txt)
{
    ui->loopList->setText(txt);

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



    cardchoicerefresh();



    hw_info data;
    data = get_device_info("hw:1,0");

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





    QString txt = "taskset -cp 3 "+n2s(QCoreApplication::applicationPid());
  //  system(txt.toStdString().c_str());

    txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());

    txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());


    txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());

    txt = "echo \"performance\" |sudo tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor";
    system(txt.toStdString().c_str());

    /*  txt = "sudo chrt -f -p 99 "+QString::number(QCoreApplication::applicationPid());
    system(txt.toStdString().c_str());


    txt = "sudo renice -n -19 -p "+QString::number(QCoreApplication::applicationPid());
    system(txt.toStdString().c_str());

*/




    mainInterface = new interface_c(this);
    mainInterface->start();

    connect(ui->bClick,SIGNAL(pressed()),mainInterface,SLOT(clickPlayStop()));


    connect(this,SIGNAL(sendKey(QKeyEvent*)),mainInterface,SLOT(keyInput(QKeyEvent*)));


    /*tester = new QTimer;
    connect(tester,SIGNAL(timeout()),mainInterface,SLOT(Test()));
    tester->start(5000);*/




}





MainWindow::~MainWindow()
{

    alsa_cleanup_playback();
    alsa_cleanup_capture();
    delete ui;


}
