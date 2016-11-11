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
#include "playback_loop_c.h"

#include "capture_loop_c.h"
#include "capture_port_c.h"


#include "qdir.h"




#define LOOP_LENGTH 1000

static playback_loop_c *pLoop0 ;
static playback_loop_c *pLoop1 ;
static playback_loop_c *pLoop2 ;
static playback_loop_c *pLoop3 ;

capture_loop_c *pCaptureLoop0;
capture_loop_c *pCaptureLoop1;
capture_loop_c *pCaptureLoop2;
capture_loop_c *pCaptureLoop3;



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

    static int num = 0;

    static bool test = true;

    num=0;


    if(test)
    {



        pCaptureLoop2->destroyLoop();
       pCaptureLoop3->destroyLoop();


        pLoop0= new playback_loop_c("loop 1_1","rec0_1.wav",pLeft,LOOP_LENGTH);
        pLoop1= new playback_loop_c("loop 1_2","rec1_1.wav",pRight,LOOP_LENGTH);

        pCaptureLoop0 = new capture_loop_c("record 0","rec0_0.wav",pRec0,CAPTURE_LOOP_BUFSIZE,NFILE_CAPTURE);
        pCaptureLoop1 = new capture_loop_c("record 1","rec1_0.wav",pRec1,CAPTURE_LOOP_BUFSIZE,NFILE_CAPTURE);





    }
    else
    {

        pCaptureLoop0->destroyLoop();
        pCaptureLoop1->destroyLoop();

        pLoop2= new playback_loop_c("loop 2_1","rec0_0.wav",pLeft,LOOP_LENGTH);
        pLoop3= new playback_loop_c("loop 2_2","rec1_0.wav",pRight,LOOP_LENGTH);

       pCaptureLoop2 = new capture_loop_c("record 2","rec0_1.wav",pRec0,CAPTURE_LOOP_BUFSIZE,NFILE_CAPTURE);
       pCaptureLoop3 = new capture_loop_c("record 3","rec1_1.wav",pRec1,CAPTURE_LOOP_BUFSIZE,NFILE_CAPTURE);


    }

    test = !test;

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





    alsa_start_playback("hw:1,0", 2, 44100);
    alsa_start_capture("hw:1,0", 2, 44100);



    QThread::sleep(0.5);

    pLeft = alsa_playback_port_by_num(0);
    pRight = alsa_playback_port_by_num(1);


    pRec0 = alsa_capture_port_by_num(0);
    pRec1 = alsa_capture_port_by_num(1);


    /*  clickTimer = new QTimer(this);
    connect(clickTimer,SIGNAL(timeout()), this, SLOT(topClick()));
    clickTimer->start(200);
*/







   pCaptureLoop2 = new capture_loop_c("record 2","rec0_0.wav",pRec0,CAPTURE_LOOP_BUFSIZE,10*NFILE_CAPTURE);
    pCaptureLoop3 = new capture_loop_c("record 3","rec1_0.wav",pRec1,CAPTURE_LOOP_BUFSIZE,10*NFILE_CAPTURE);
   QThread::sleep(2);
    pLoop0= new playback_loop_c("loop 1_1","rec0_0.wav",pLeft,10*LOOP_LENGTH);
    pLoop1= new playback_loop_c("loop 1_2","rec1_0.wav",pRight,10*LOOP_LENGTH);






       // topStep();



    return;


    stepTimer = new QTimer(this);
    connect(stepTimer,SIGNAL(timeout()), this, SLOT(topStep()));
    stepTimer->start(LOOP_LENGTH);













    return;

}








MainWindow::~MainWindow()
{

    alsa_cleanup_playback();
    alsa_cleanup_capture();
    delete ui;


}
