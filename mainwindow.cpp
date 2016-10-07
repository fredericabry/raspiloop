#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include "alsa_playback.h"
#include <stdbool.h>


#include <QFile>
#include <QTextStream>

#include <qdebug.h>
#include <qtimer.h>







RINGBUF_DEF(main_buf_playback,20000);




QStringList getCardDescription(void)
{

    QStringList liste;
    return liste;
}






void MainWindow::connectToCard(void)
{
}




void MainWindow::link()
{
}






void MainWindow::chooseCard(void)
{
    int numout = ui->cardchoice->currentIndex();
    int numin = ui->cardchoice_input->currentIndex();
    InputCardName = InputCardNames.at(numin);
    OutputCardName = OutputCardNames.at(numout);



    /*Afficher("-------------\nPlayback : "+OutputCardLongNames.at(numout)+"\n");
Afficher(info(OutputCardName, SND_PCM_STREAM_PLAYBACK) );
Afficher("Capture : "+InputCardLongNames.at(numin)+"\n");
Afficher(info(InputCardName, SND_PCM_STREAM_CAPTURE) );
*/



    return;


    //Afficher(param);

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
    ringbuf_fill(&main_buf_playback);
}



void MainWindow::topClick()
{

    static int x = 0;
if(ui->radioClick->isChecked())
{
    x++;
    if(x>4)
        x=1;


    if(x==1) alsa_play("hw:1,0",2,44100,2000,"ding.wav",this);
    else alsa_play("hw:1,0",2,44100,2000,"ding2.wav",this);



}

}


void MainWindow::updateTempo(int tempo)
{

return;
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






/*    clickTimer = new QTimer(this);
    connect(clickTimer,SIGNAL(timeout()), this, SLOT(topClick()));
    clickTimer->start(1000);
   // alsa_record("hw:1,0",2,44100,2000,"test.wav",this);
*/
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

*/

    short *dt;
    dt = (short*)malloc(5*sizeof(short));
    dt[0] = 2;
    dt[1] = 4000;
    dt[2] = 6;
    dt[3] = 8000;
    dt[4] = 10;

    RINGBUF_DEF(buf,5);
    buf.head = 4;
    buf.tail = 3;

    AfficherI(ringbuf_freespace(&buf));

    ringbuf_pushN(&buf, dt , 4);


    AfficherI(buf.head);
    AfficherI(buf.tail);
br();


    //ringbuf_push(&buf,dt[1]);
    short x;
    for(int i = 0;i<6;i++)
    {
                AfficherI(buf.buf[i]);
    }
br();
    for(int i = 0;i<5;i++)
    {
          if(ringbuf_pull(&buf,&x) != -1)
           AfficherI(x);
           else
               Afficher("no data");

    }
    br();
    AfficherI(buf.head);
    AfficherI(buf.tail);

return;

    AfficherI(buf.head);



    AfficherI(ringbuf_length(&buf));






    //alsa_start_playback("hw:1,0", 2, 44100,this,&main_buf_playback);



}








MainWindow::~MainWindow()
{
    delete ui;
}
