#ifndef DIALOG_DEVICE_H
#define DIALOG_DEVICE_H

#include <QDialog>
#include "qstringlist.h"
#include "QSignalMapper"

class QMainWindow;

class interface_c;

namespace Ui {
class dialog_device;
}

class dialog_device : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_device(QWidget *parent = 0,QMainWindow* mainwindow = NULL, interface_c *pInterface =0, int type=0);
    ~dialog_device();
    void draw(void);

private:

    QMainWindow* mainwindow;
    Ui::dialog_device *ui;
    int type;
    interface_c *pInterface;
    QSignalMapper *signalMapper;
    unsigned int devicesCount;
    QStringList cardNames,cardLongNames;
    QString keyWord;
    std::vector<QPushButton*> buttonsList;

private slots:
    void refresh(void);



};

#endif
