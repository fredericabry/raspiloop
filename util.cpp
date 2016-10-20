#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextStream>


void MainWindow::br()
{
    Afficher("\n");
}

void MainWindow::Afficher(QString txt)
{


    ui->console->append(txt);

}


void MainWindow::AfficherListe(QStringList liste)
{

    int n = liste.length();

    for (int i = 0; i < n;i++)
    {
        Afficher(liste.at(i));


    }


}



void MainWindow::consoleclear()
{
    ui->console->clear();
}


void MainWindow::AfficherI(int i)
{
    Afficher(n2s(i));
}


