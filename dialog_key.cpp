#include "dialog_key.h"
#include "ui_dialog_key.h"



dialog_key::dialog_key(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_key)
{
    ui->setupUi(this);
}

dialog_key::~dialog_key()
{
    ((QWidget*)parent())->setEnabled(true);
    delete ui;
}


void dialog_key::keyPressEvent(QKeyEvent *e)
{
    if ((e->key()==Qt::Key_CapsLock)
            ||(e->key()==Qt::Key_NumLock))
        return;

    emit sendKey(e);

    deleteLater();
}
