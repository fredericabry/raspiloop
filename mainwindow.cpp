#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alsa_util.h"
#include <stdbool.h>


#include <QFile>
#include <QTextStream>

#include <qdebug.h>












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







MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{




    ui->setupUi(this);


    connect(ui->brefresh,SIGNAL(pressed()),this,SLOT(cardchoicerefresh()));
    connect(ui->bconnect,SIGNAL(pressed()),this,SLOT(chooseCard()));
    connect(ui->bclear,SIGNAL(pressed()),this,SLOT(consoleclear()));



    Afficher("Démarrage\n");




    cardchoicerefresh();



    alsa_record("hw:1,0",2,44100,3000,"test.wav",this);




}









MainWindow::~MainWindow()
{
    delete ui;
}
