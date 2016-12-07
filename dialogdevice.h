#ifndef DIALOGDEVICE_H
#define DIALOGDEVICE_H

#include <QDialog>
#include "qstringlist.h"
#include "QSignalMapper"
class interface_c;

namespace Ui {
class DialogDevice;
}

class DialogDevice : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDevice(QWidget *parent = 0, interface_c *pInterface =0, int type=0);
    ~DialogDevice();


private:
    Ui::DialogDevice *ui;
    interface_c *pInterface;
    QSignalMapper *signalMapper;
    unsigned int devicesCount;
    QStringList cardNames,cardLongNames;
    QString keyWord;
    std::vector<QPushButton*> buttonsList;

private slots:
    void clicked(const QString &text);



};

#endif // DIALOGDEVICE_H
