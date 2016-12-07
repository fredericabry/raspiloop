#include "parameterdialog.h"
#include "ui_parameterdialog.h"

ParameterDialog::ParameterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParameterDialog)
{
    ui->setupUi(this);
        connect(this->ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));
}

ParameterDialog::~ParameterDialog()
{

    delete ui;
}
