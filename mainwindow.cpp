#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include "alsa_capture.h"
#include <stdbool.h>


#include <QFile>
#include <QTextStream>

#include <qdebug.h>
#include <qtimer.h>
#include <qelapsedtimer.h>

#include "playback_port_c.h"
#include "loop_c.h"




playback_port_c *pLeft,*pRight;

capture_port_c *pRec0, *pRec1;


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

    unsigned long a;
    alsa_monitor_playback(0,&a);
    ui->playback_freespace->display((int)a);
  //  alsa_monitor_capture(0,&a);
  //  ui->capture_freespace->display((int)a);
    return;

    static int x = 0;
    if(ui->radioClick->isChecked())
    {
        x++;
        if(x>4)
            x=1;


        if((x==1) || (x == 3))
        {



        }

        else

        {


        }


    }

}


void MainWindow::updateTempo(int tempo)
{

    clickTimer->start(1000*60/tempo);

}


void MainWindow::record(void)
{
    return;

    pRec0 = alsa_capture_port_by_num(0);
    pRec1 = alsa_capture_port_by_num(1);
    if(pRec0->recording)
    {
        pRec0->stoprecord();
        pRec1->stoprecord();
        play();

    }
    else
    {

        pRec0->startrecord("rec0.wav");
        pRec1->startrecord("rec1.wav");
    }

}
void MainWindow::play()
{
static bool playing = false;
static loop_c *pLoop0 ;
static loop_c *pLoop1 ;


if(!playing)
{


    pLoop0= new loop_c;
    pLoop1= new loop_c;


    pLoop0->init("rec0.wav",pLeft,-1);
    pLoop1->init("rec1.wav",pRight,-1);

    ui->bPlay->setText("Stop !");

}
else
{
    pLoop0->destroyloop();
    pLoop1->destroyloop();
    ui->bPlay->setText("Play !");


}


playing =!playing;


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













    alsa_start_playback("hw:1,0", 2, 44100);
    alsa_start_capture("hw:1,1", 2, 44100);


    pLeft = alsa_playback_port_by_num(0);
    pRight = alsa_playback_port_by_num(1);


    clickTimer = new QTimer(this);
    connect(clickTimer,SIGNAL(timeout()), this, SLOT(topClick()));
    clickTimer->start(500);





}








MainWindow::~MainWindow()
{

    alsa_cleanup_playback();
    alsa_cleanup_capture();
    delete ui;


}
