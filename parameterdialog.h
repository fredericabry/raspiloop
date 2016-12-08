#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>

class interface_c;



namespace Ui {
class ParameterDialog;
}

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterDialog(QWidget *parent = 0, interface_c *pInterface=NULL);
    ~ParameterDialog();
    void setupCLickButtons(void);

    int mixLoopNumber;
private:
    Ui::ParameterDialog *ui;
    interface_c *pInterface;
    std::vector<QPushButton *> buttonList;


public slots:
    void bMixAuto(void);
    void bMixPreset(void);
    void bMixPresetPlus(void);
    void bMixPresetMinus(void);

};

#endif // PARAMETERDIALOG_H
