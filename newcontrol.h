#ifndef NEWCONTROL_H
#define NEWCONTROL_H

#include <QMainWindow>

namespace Ui {
class newcontrol;
}

class newcontrol : public QMainWindow
{
    Q_OBJECT

public:
    explicit newcontrol(QWidget *parent = 0);
    ~newcontrol();

private:
    Ui::newcontrol *ui;
};

#endif // NEWCONTROL_H
