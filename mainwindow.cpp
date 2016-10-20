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

FILE *debugfile;

static loop_c *pLoop0 ;
static loop_c *pLoop1 ;
static loop_c *pLoop2 ;
static loop_c *pLoop3 ;



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
    alsa_monitor_capture(0,&a);
    ui->capture_freespace->display((int)a);
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


    static bool record = false;




    pRec0 = alsa_capture_port_by_num(0);
    pRec1 = alsa_capture_port_by_num(1);



    if(record)
    {
        pRec0->stoprecord();
        pRec1->stoprecord();
        record = false;


    }

    else

    {

        pRec0->startrecord("rec0.wav");
        pRec1->startrecord("rec1.wav");
        record = true;
    }





}





void MainWindow::play()
{

    static int num = 0;

    static bool test = true;

    num=1;


    if(test)
    {

        pLoop0= new loop_c("loop 0","rec0_"+n2s(num-1)+".wav",pLeft,500);
        pLoop1= new loop_c("loop 1","rec1_"+n2s(num-1)+".wav",pRight,500);

        pRec0->stoprecord();
       pRec1->stoprecord();
     //   sleep(0);

       pRec0->startrecord("rec0_"+n2s(num)+".wav");
       pRec1->startrecord("rec1_"+n2s(num)+".wav");


    }
   else
    {

        pLoop2= new loop_c("loop 0","rec0_"+n2s(num)+".wav",pLeft,500);
        pLoop3= new loop_c("loop 1","rec1_"+n2s(num)+".wav",pRight,500);

        pRec0->stoprecord();
        pRec1->stoprecord();


        pRec0->startrecord("rec0_"+n2s(num-1)+".wav");
        pRec1->startrecord("rec1_"+n2s(num-1)+".wav");


    }

    test = !test;

}






void debugf(QString txt)
{

    fprintf(debugfile,"%s\n",txt.toStdString().c_str());

}




void MainWindow::topStep()
{
    static int step =0;


    step++;

    qDebug()<<"loop "<<step;
    play();


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

    debugfile = fopen("debufile.txt","w");

    debugf("Init");

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



    QThread::sleep(0.5);

    pLeft = alsa_playback_port_by_num(0);
    pRight = alsa_playback_port_by_num(1);



    pLoop2= new loop_c("loop 0","rec0_0.wav",pLeft,0);
    pLoop3= new loop_c("loop 1","rec1_0.wav",pRight,0);




    pRec0 = alsa_capture_port_by_num(0);
   pRec1 = alsa_capture_port_by_num(1);

    clickTimer = new QTimer(this);
    connect(clickTimer,SIGNAL(timeout()), this, SLOT(topClick()));
    clickTimer->start(500);


    topStep();

    stepTimer = new QTimer(this);
    connect(stepTimer,SIGNAL(timeout()), this, SLOT(topStep()));
    stepTimer->start(500);


}








MainWindow::~MainWindow()
{

    alsa_cleanup_playback();
    alsa_cleanup_capture();
    delete ui;


}
