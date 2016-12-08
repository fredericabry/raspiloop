#include "newcontrol.h"
#include "ui_newcontrol.h"

newcontrol::newcontrol(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::newcontrol)
{
    ui->setupUi(this);
}

newcontrol::~newcontrol()
{
    delete ui;
}
