#ifndef DIALOG_KEYLIST_H
#define DIALOG_KEYLIST_H

#include <QDialog>
#include "qpushbutton.h"
#include "interface.h"

namespace Ui {
class dialog_keylist;
}

class dialog_keylist : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_keylist(QWidget *parent = 0, interface_c *pInterface = NULL);
    ~dialog_keylist();
    void draw();
private:
    Ui::dialog_keylist *ui;
    interface_c *pInterface;

public slots:
    void clickButton(void)    ;
    void removeAll(void);
};

#endif // DIALOG_KEYLIST_H
