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

void MainWindow::ding()
{
    playback_port_c *pRing = alsa_playback_port_by_num(0);
    loop_c *pLoop2 = new loop_c;
    pLoop2->init("grosse tec.wav",pRing,15*44100);


    playback_port_c *pRing2 = alsa_playback_port_by_num(1);
    pLoop2 = new loop_c;
    pLoop2->init("grosse tec.wav",pRing2,15*44100);


}



void MainWindow::topClick()
{

    unsigned long a;
    alsa_monito(&a);
    ui->freespace->display((int)a);

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






MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{




    ui->setupUi(this);


    connect(ui->brefresh,SIGNAL(pressed()),this,SLOT(cardchoicerefresh()));
    connect(ui->bconnect,SIGNAL(pressed()),this,SLOT(chooseCard()));
    connect(ui->bclear,SIGNAL(pressed()),this,SLOT(consoleclear()));
    connect(ui->b_ding,SIGNAL(pressed()),this,SLOT(ding()));



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

*/








    alsa_start_playback("hw:1,0", 2, 44100);
    alsa_start_capture("hw:1,0", 2, 44100);



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
