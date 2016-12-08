#include "parameterdialog.h"
#include "ui_parameterdialog.h"
#include "interface.h"
#include "playback_port_c.h"
#include "config_file.h"

ParameterDialog::ParameterDialog(QWidget *parent,interface_c *pInterface) :
    QDialog(parent),
    ui(new Ui::ParameterDialog),
    pInterface(pInterface)
{
    ui->setupUi(this);
    connect(this->ui->bClose,SIGNAL(pressed()),this,SLOT(deleteLater()));
    connect(this->ui->bAuto,SIGNAL(pressed()),this,SLOT(bMixAuto()));
    connect(this->ui->bPreset,SIGNAL(pressed()),this,SLOT(bMixPreset()));
    connect(this->ui->bPresetPlus,SIGNAL(pressed()),this,SLOT(bMixPresetPlus()));
    connect(this->ui->bPresetMinus,SIGNAL(pressed()),this,SLOT(bMixPresetMinus()));



    setupCLickButtons();


    QStringList mix,mix_preset;

    extractParameter(KEYWORD_MIX_STRATEGY, &mix);
    extractParameter(KEYWORD_MIX_PRESET_NUMBER, &mix_preset);

    ui->bPresetMinus->setEnabled(false);
    ui->bPresetPlus->setEnabled(false);
    ui->bAuto->setCheckable(true);
    ui->bPreset->setCheckable(true);

    if(mix.size()>0) //we only use [0] value
    {

        if(mix[0] == MIX_STRATEGY_PRESET)
        {
            ui->bPresetMinus->setEnabled(true);
            ui->bPresetPlus->setEnabled(true);
            ui->bPreset->setChecked(true);
            ui->bAuto->setChecked(false);
        }
        else if(mix[0] == MIX_STRATEGY_AUTO)
        {
            ui->bPresetMinus->setEnabled(false);
            ui->bPresetPlus->setEnabled(false);
            ui->bPreset->setChecked(false);
            ui->bAuto->setChecked(true);

        }
    }


    bool test = true;

    if(mix_preset.size()>0)
    {

        mixLoopNumber = mix_preset[0].toInt(&test);
        if(!test) mixLoopNumber = 1;//error
        if(mixLoopNumber<1) mixLoopNumber = 1;
        if(mixLoopNumber>100) mixLoopNumber = 100;
    }
    else mixLoopNumber = 1;





    ui->bPreset->setText("Preset\n"+QString::number(mixLoopNumber)+" loops");






}









void ParameterDialog::setupCLickButtons(void)
{

    QStringList portNamesFile;
    extractParameter(KEYWORD_CLICK_PORTS, &portNamesFile);
    int i = 0;
    for (auto &pPort : pInterface->playbackPortsList)
    {

        QPushButton *portButton = new QPushButton(this);
        QFont font;
        font.setPointSize(6);
        portButton->setFont(font);
        portButton->setText("#"+QString::number(pPort->channel));
        QRect pos = ui->lClick->geometry();
        portButton->setGeometry(pos);

        pos.moveTo(pos.x()+100+100*i,pos.y());
        portButton->setGeometry(pos);
        portButton->setCheckable(true);


        if(portNamesFile.contains(QString::number(pPort->channel))) //device is in config file already
        {
            portButton->setChecked(true);

        }

        i++;

        buttonList.push_back(portButton);
    }

}

void ParameterDialog::bMixAuto(void)
{

        ui->bPreset->setChecked(ui->bAuto->isChecked());
        ui->bPresetPlus->setEnabled(ui->bAuto->isChecked());
        ui->bPresetMinus->setEnabled(ui->bAuto->isChecked());


}

void ParameterDialog::bMixPreset(void)
{
    ui->bPresetPlus->setEnabled(!ui->bPreset->isChecked());
    ui->bPresetMinus->setEnabled(!ui->bPreset->isChecked());
    ui->bAuto->setChecked(ui->bPreset->isChecked());
}
void ParameterDialog::bMixPresetPlus(void)
{
if(mixLoopNumber<100) mixLoopNumber++;
ui->bPreset->setText("Preset\n"+QString::number(mixLoopNumber)+" loops");


}
void ParameterDialog::bMixPresetMinus(void)
{
    if(mixLoopNumber>1) mixLoopNumber--;
    ui->bPreset->setText("Preset\n"+QString::number(mixLoopNumber)+" loops");

}






ParameterDialog::~ParameterDialog()
{

    //click ports

    QStringList nuParams;
    std::vector<playback_port_c*> pNuPorts;


    for (unsigned int i=0;i<buttonList.size();i++)
    {
        if(buttonList[i]->isChecked())
        {
            nuParams.append(QString::number(i+1));
            pNuPorts.push_back(pInterface->playbackPortsList[i]);
        }
    }


    setParameter(KEYWORD_CLICK_PORTS,nuParams,true);
    pInterface->moveClick(pNuPorts);



    //mix strategy

    if(ui->bAuto->isChecked())
    {
        setParameter(KEYWORD_MIX_STRATEGY,(QStringList)MIX_STRATEGY_AUTO,true);
        pInterface->setMixStrategy(AUTO);
    }
    else if (ui->bPreset->isChecked())
    {
        setParameter(KEYWORD_MIX_STRATEGY,(QStringList)MIX_STRATEGY_PRESET,true);
        setParameter(KEYWORD_MIX_PRESET_NUMBER,(QStringList)(QString::number(mixLoopNumber)),true);
        pInterface->setMixStrategy(PRESET);
        pInterface->setMixLoopNumber(mixLoopNumber);
    }




    ((QMainWindow*)parent())->setEnabled(true);






    delete ui;
}




