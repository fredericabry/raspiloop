#ifndef DIALOG_CONTROLLIST_H
#define DIALOG_CONTROLLIST_H





#include <QDialog>


class interface_c;
class QMainWindow;





namespace Ui {
class dialog_controllist;
}

class dialog_controllist : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_controllist(QWidget *parent = 0,QMainWindow* mainwindow = NULL, interface_c *pInterface =0);
    ~dialog_controllist();
    void openNumeric(void);

private:
    QMainWindow *mainwindow;
     Ui::dialog_controllist *ui;
    interface_c *pInterface;
    QString output;
public slots:
    void pushButton(void);
    void pushButtonInt(void);
    void getNumericData(int data);

signals:
    void sendText(QString txt);//to signal the chosen control

};

#endif // DIALOG_CONTROLLIST_H
