#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>

namespace Ui {
class ParameterDialog;
}

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterDialog(QWidget *parent = 0);
    ~ParameterDialog();

private:
    Ui::ParameterDialog *ui;
};

#endif // PARAMETERDIALOG_H
