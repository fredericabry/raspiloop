#ifndef DIALOG_NUMERIC_H
#define DIALOG_NUMERIC_H

#include <QDialog>

namespace Ui {
class dialog_numeric;
}

class dialog_numeric : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_numeric(QWidget *parent = 0);
    ~dialog_numeric();
signals:
    void exportData(int);
public slots:
    void b0(void);
    void b1(void);
    void b2(void);
    void b3(void);
    void b4(void);
    void b5(void);
    void b6(void);
    void b7(void);
    void b8(void);
    void b9(void);
    void bC(void);
private:
    Ui::dialog_numeric *ui;
    int value;
    QString text;
};

#endif // DIALOG_NUMERIC_H
