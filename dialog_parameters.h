#ifndef dialog_parameters_H
#define dialog_parameters_H

#include <QDialog>

class interface_c;



namespace Ui {
class dialog_parameters;
}

class dialog_parameters : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_parameters(QWidget *parent = 0, interface_c *pInterface=NULL);
    ~dialog_parameters();
    void setupCLickButtons(void);


    int mixLoopNumber;
private:
    Ui::dialog_parameters *ui;
    interface_c *pInterface;
    std::vector<QPushButton *> buttonList;


public slots:
    void bMixAuto(void);
    void bMixPreset(void);
    void bMixPresetPlus(void);
    void bMixPresetMinus(void);
    void dialogInputDevice(void);
    void dialogOutputDevice(void);
    void dialogMidiDevice(void);
    void newControl();
    void showControlList();
    void deviceRestart();
};

#endif // dialog_parameters_H
