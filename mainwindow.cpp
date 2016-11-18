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
    int numout = ui->cardchoice->currentIndex();
    int numin = ui->cardchoice_input->currentIndex();
    InputCardName = InputCardNames.at(numin);
    OutputCardName = OutputCardNames.at(numout);
}

void MainWindow::cardchoicerefresh(void)
{
    getCardList(SND_PCM_STREAM_PLAYBACK,&OutputCardNames,&OutputCardLongNames);
    getCardList(SND_PCM_STREAM_CAPTURE,&InputCardNames,&InputCardLongNames);

    ui->cardchoice->clear();
    ui->cardchoice->addItems(OutputCardLongNames);
    ui->cardchoice_input->clear();
    ui->cardchoice_input->addItems(InputCardLongNames);
}







void MainWindow::topClick()
{


}


void MainWindow::updateTempo(int tempo)
{

    clickTimer->start(1000*60/tempo);

}


void MainWindow::record(void)
{




}

void MainWindow::play()
{


}

void MainWindow::shutdown()
{

    exit(0);

}

void MainWindow::topStep()
{
    static int step =0;


    step++;

    if(step%10==0) qDebug()<<"loop "<<step;



    play();


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
    connect(ui->brefresh,SIGNAL(pressed()),this,SLOT(cardchoicerefresh()));
    connect(ui->bconnect,SIGNAL(pressed()),this,SLOT(chooseCard()));
    connect(ui->bclear,SIGNAL(pressed()),this,SLOT(consoleclear()));
    connect(ui->bPlay,SIGNAL(pressed()),this,SLOT(play()));
    connect(ui->bRecord,SIGNAL(pressed()),this,SLOT(record()));

    connect(ui->b_shutdown,SIGNAL(pressed()),this,SLOT(shutdown()));


    Afficher("Démarrage\n");


    cardchoicerefresh();

    connect(ui->tempoBox,SIGNAL(valueChanged(int)),this,SLOT(updateTempo(int)));

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
    connect(this,SIGNAL(sendKey(QKeyEvent*)),mainInterface,SLOT(keyInput(QKeyEvent*)));



}





MainWindow::~MainWindow()
{

    alsa_cleanup_playback();
    alsa_cleanup_capture();
    delete ui;


}
