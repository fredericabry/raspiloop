#include "dialog_numeric.h"
#include "ui_dialog_numeric.h"

dialog_numeric::dialog_numeric(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_numeric)
{
    ui->setupUi(this);
    connect(ui->bOk,SIGNAL(pressed()),this,SLOT(deleteLater()));
    connect(ui->b0,SIGNAL(pressed()),this,SLOT(b0()));
    connect(ui->b1,SIGNAL(pressed()),this,SLOT(b1()));
    connect(ui->b2,SIGNAL(pressed()),this,SLOT(b2()));
    connect(ui->b3,SIGNAL(pressed()),this,SLOT(b3()));
    connect(ui->b4,SIGNAL(pressed()),this,SLOT(b4()));
    connect(ui->b5,SIGNAL(pressed()),this,SLOT(b5()));
    connect(ui->b6,SIGNAL(pressed()),this,SLOT(b6()));
    connect(ui->b7,SIGNAL(pressed()),this,SLOT(b7()));
    connect(ui->b8,SIGNAL(pressed()),this,SLOT(b8()));
    connect(ui->b9,SIGNAL(pressed()),this,SLOT(b9()));
    connect(ui->bC,SIGNAL(pressed()),this,SLOT(bC()));
    connect(ui->bCC,SIGNAL(pressed()),this,SLOT(bCC()));

    text="";
    ui->console->setText(text);
}

dialog_numeric::~dialog_numeric()
{
    bool test = true;
    value = text.toInt(&test);
    if(!test) value = 0;

    emit exportData(value);
    ((QWidget*)parent())->setEnabled(true);
    delete ui;
}



void dialog_numeric::b0(void)
{
    if(text.size()<3)
        text.append("0");
    ui->console->setText(text);
}
void dialog_numeric::b1(void)
{
    if(text.size()<3)
        text.append("1");
    ui->console->setText(text);
}

void dialog_numeric::b2(void)
{
    if(text.size()<3)
        text.append("2");
    ui->console->setText(text);
}
void dialog_numeric::b3(void)
{
    if(text.size()<3)
        text.append("3");
    ui->console->setText(text);
}
void dialog_numeric::b4(void)
{
    if(text.size()<3)
        text.append("4");
    ui->console->setText(text);
}
void dialog_numeric::b5(void)
{
    if(text.size()<3)
        text.append("5");
    ui->console->setText(text);
}
void dialog_numeric::b6(void)
{
    if(text.size()<3)
        text.append("6");
    ui->console->setText(text);
}

void dialog_numeric::b7(void)
{
    if(text.size()<3)
        text.append("7");
    ui->console->setText(text);
}
void dialog_numeric::b8(void)
{
    if(text.size()<3)
        text.append("8");
    ui->console->setText(text);
}
void dialog_numeric::b9(void)
{

    if(text.size()<3)
        text.append("9");
    ui->console->setText(text);
}
void dialog_numeric::bC()
{
    if(text.size()>0)
        text.remove(text.length()-1,1);
    ui->console->setText(text);
}

void dialog_numeric::bCC()
{

    text = "-1";
    ui->console->setText("midi CC");
    deleteLater();
}
