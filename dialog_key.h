#ifndef DIALOG_KEY_H
#define DIALOG_KEY_H

#include <QDialog>
#include "qobject.h"
#include "QKeyEvent"


namespace Ui {
class dialog_key;
}

class dialog_key : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_key(QWidget *parent = 0);
    void keyPressEvent(QKeyEvent *e);
    ~dialog_key();

private:
    Ui::dialog_key *ui;

signals:
    void sendKey(QKeyEvent *e);
};

#endif // DIALOG_KEY_H
