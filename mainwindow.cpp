#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>


#include <QFile>
#include <QTextStream>

#include <qdebug.h>
#include <qtimer.h>
#include <qelapsedtimer.h>

#include "ringbuf_c.h"














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

    //alsa_play("hw:1,0",2,44100,2000,"ding.wav",this);
    alsa_load_file(0);

    //ringbuf_fill(&left_buf_playback);
}



void MainWindow::topClick()
{

    static int x = 0;
    if(ui->radioClick->isChecked())
    {
        x++;
        if(x>4)
            x=1;


        if((x==1) || (x == 3))
        {
alsa_load_file(1);


        }

        else

        {
alsa_load_file(1);

        }


    }

}


void MainWindow::updateTempo(int tempo)
{

    clickTimer->start(1000*60/tempo);

}


void MainWindow::click_change(bool val)
{
  // AfficherI(val);

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

    connect(ui->radioClick,SIGNAL(clicked(bool)),this,SLOT(click_change(bool)));

    Afficher("Démarrage\n");




    cardchoicerefresh();









    connect(ui->tempoBox,SIGNAL(valueChanged(int)),this,SLOT(updateTempo(int)));


    hw_info data;

    data = get_device_info("hw:1,0");


    Afficher(data.name);
    Afficher("max playback "+n2s(data.max_playback));
    Afficher("min playback "+n2s(data.min_playback));
    Afficher("max capture "+n2s(data.max_capture));
    Afficher("min capture "+n2s(data.min_capture));
    Afficher("max rate "+n2s(data.max_rate) + "Hz");
    Afficher("min rate "+n2s(data.min_rate) + "Hz");
    Afficher("max buffer" + n2s(data.max_buffer_size));
    Afficher("min buffer" + n2s(data.min_buffer_size));











    alsa_start_playback("hw:1,0", 2, 44100,this);




    clickTimer = new QTimer(this);
  connect(clickTimer,SIGNAL(timeout()), this, SLOT(topClick()));
  clickTimer->start(500);


 alsa_start_file("grosse tec.wav",0);


}








MainWindow::~MainWindow()
{
    delete ui;
}
