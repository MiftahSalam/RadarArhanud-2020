#include "frameleft.h"
#include "ui_frameleft.h"
#include "echo/radar_global.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QDesktopWidget>

FrameLeft::FrameLeft(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameLeft)
{
    ui->setupUi(this);

    ui->checkBoxShowRing->setChecked(radar_settings.show_rings);
    ui->checkBoxShowMap->setChecked(map_settings.show);
    ui->comboBoxMapMode->setCurrentIndex((int)map_settings.mode);
    ui->checkBoxShowHM->setChecked(radar_settings.show_heading_marker);
    ui->checkBoxShowCompass->setChecked(radar_settings.show_compass);
    ui->checkBoxMTI->setChecked(mti_settings.enable);
    ui->horizontalSliderMTI->setValue(mti_settings.threshold);
    ui->lineEditMTI->setText(QString::number(mti_settings.threshold));

    ui->groupBoxSubSistemStatus->hide();

    dRadar = new DialogRadar(this);
    dIFF = new DialogIFF(this);
    dADSB = new DialogADSB(this);
    dTrail = new TrailDialog(this);
    dLog = new DialogLogging(this);

    dRadar->setModal(true);
    dIFF->setModal(true);
    dADSB->setModal(true);
    dTrail->setModal(true);
    dLog->setModal(true);

    ui->labelRange->setText("1.5 NM");

    connect(dRadar,SIGNAL(signal_settingChange()),this,SIGNAL(signal_radarSettingChange()));
//    state_radar = RADAR_TRANSMIT; //temporary
    trigger_stateChange();
}

void FrameLeft::setRangeRings(qreal range)
{
    ui->labelRingRange->setText(QString::number(range,'f',2)+" NM");
}

void FrameLeft::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug()<<Q_FUNC_INFO;

    QMenu menu;

    menu.addAction("Exit",this,SIGNAL(signal_exit()));
    menu.exec(event->pos());
}

FrameLeft::~FrameLeft()
{
    delete ui;
}

void FrameLeft::trigger_newLog(QString msg)
{
    dLog->insertLog(msg);
}

void FrameLeft::on_checkBoxShowRing_clicked(bool checked)
{
    radar_settings.show_rings = checked;
}

void FrameLeft::on_checkBoxShowMap_clicked(bool checked)
{
    map_settings.show = checked;
    emit signal_mapChange(0,checked);
}

void FrameLeft::on_comboBoxMapMode_activated(int index)
{
    map_settings.mode = (quint8)index;
    emit signal_mapChange(1,index);
}

void FrameLeft::on_checkBoxShowHM_clicked(bool checked)
{
    radar_settings.show_heading_marker = checked;
}

void FrameLeft::on_checkBoxShowCompass_clicked(bool checked)
{
    radar_settings.show_compass = checked;
}

void FrameLeft::on_pushButtonSetRadar_clicked()
{
    dRadar->show();
}

void FrameLeft::on_pushButtonSetTrail_clicked()
{
    dTrail->show();
}

void FrameLeft::on_pushButtonSetIFF_clicked()
{
    dIFF->show();
}

void FrameLeft::on_pushButtonSetADSB_clicked()
{
    dADSB->show();
}

void FrameLeft::on_pushButtonLogger_clicked()
{
    dLog->show();
}

void FrameLeft::on_pushButtonTxStnb_clicked()
{
    if(ui->pushButtonTxStnb->text() == "Transmit")
    {
        int g;
        QString rngName = ui->labelRange->text();
        for (g = ARRAY_SIZE(g_ranges_metric); g > 0; g--)
        {
            if (QString(g_ranges_metric[g].name )== rngName)
                break;
        }
        if(g < 0)
            g = 0;

        emit signal_req_range(g_ranges_metric[g].meters);
        emit signal_Tx();
    }
    else if(ui->pushButtonTxStnb->text() == "Standby")
        emit signal_Standby();
}

void FrameLeft::trigger_stateChange()
{
    qDebug()<<Q_FUNC_INFO<<(int)state_radar;

    if((state_radar == RADAR_OFF) || (state_radar == RADAR_WAKING_UP))
    {
        ui->pushButtonTxStnb->setText("Tx/Stby");
        ui->pushButtonGain->setEnabled(false);
        ui->pushButtonRain->setEnabled(false);
        ui->horizontalSliderRain->setEnabled(false);
        ui->horizontalSliderGain->setEnabled(false);
        ui->pushButtonTxStnb->setEnabled(false);
    }
    else if(state_radar == RADAR_STANDBY)
    {
        ui->pushButtonTxStnb->setText("Transmit");
        ui->pushButtonGain->setEnabled(true);
        ui->pushButtonRain->setEnabled(true);
        ui->horizontalSliderRain->setEnabled(true);
        ui->horizontalSliderGain->setEnabled(true);
        ui->pushButtonTxStnb->setEnabled(true);
    }
    else if(state_radar == RADAR_TRANSMIT)
    {
        ui->pushButtonTxStnb->setText("Standby");
        ui->pushButtonGain->setEnabled(true);
        ui->pushButtonRain->setEnabled(true);
        ui->horizontalSliderRain->setEnabled(true);
        ui->horizontalSliderGain->setEnabled(true);
        ui->pushButtonTxStnb->setEnabled(true);
    }
}

void FrameLeft::trigger_reportChange()
{
//    qDebug()<<Q_FUNC_INFO<<filter.gain<<filter.rain<<filter.sea;
//    ui->horizontalSliderGain->setValue(filter.gain);
//    ui->horizontalSliderRain->setValue(filter.rain);
    ui->lineEditGain->setText(QString::number(filter.gain));
    ui->lineEditRain->setText(QString::number(filter.rain));
}

void FrameLeft::on_pushButtonZoomIn_clicked()
{
    qDebug()<<Q_FUNC_INFO<<ui->labelRange->text();

    int g;
    QString rngName = ui->labelRange->text();
    for (g = ARRAY_SIZE(g_ranges_metric); g > 0; g--)
    {
        if (QString(g_ranges_metric[g].name )== rngName)
            break;
    }
    g--;
    if(g < 0)
        g = 0;

    ui->labelRange->setText(g_ranges_metric[g].name);
    emit signal_req_range(g_ranges_metric[g].meters);
}

void FrameLeft::setRangeText(int range)
{
    qDebug()<<Q_FUNC_INFO;
    int g;
    for (g = 0; g < ARRAY_SIZE(g_ranges_metric); g++)
    {
        if (QString(g_ranges_metric[g].meters )== range)
            break;
    }
    if(g >= ARRAY_SIZE(g_ranges_metric))
        g--;

    ui->labelRange->setText(g_ranges_metric[g].name);
}

void FrameLeft::on_pushButtonZoomOut_clicked()
{
    qDebug()<<Q_FUNC_INFO;

    int g;
    QString rngName = ui->labelRange->text();
    for (g = 0; g < ARRAY_SIZE(g_ranges_metric); g++)
    {
        if (QString(g_ranges_metric[g].name )== rngName)
            break;
    }
    g++;
    if(g >= ARRAY_SIZE(g_ranges_metric))
        g--;

    ui->labelRange->setText(g_ranges_metric[g].name);
    emit signal_req_range(g_ranges_metric[g].meters);
}

void FrameLeft::on_pushButtonGain_clicked()
{
    QString cur_text = ui->pushButtonGain->text();
    if(cur_text.contains("Manual"))
    {
        cur_text.replace("Manual","Auto");
        ui->horizontalSliderGain->setEnabled(false);
        emit signal_req_control(CT_GAIN,-1);
    }
    else
    {
        cur_text.replace("Auto","Manual");
        ui->horizontalSliderGain->setEnabled(true);
        emit signal_req_control(CT_GAIN,10);
    }
    ui->pushButtonGain->setText(cur_text);
}

void FrameLeft::on_horizontalSliderGain_valueChanged(int value)
{
    emit signal_req_control(CT_GAIN,value);
}

void FrameLeft::on_pushButtonRain_clicked()
{
    QString cur_text = ui->pushButtonRain->text();
    if(cur_text.contains("Manual"))
    {
        cur_text.replace("Manual","Auto");
        ui->horizontalSliderRain->setEnabled(false);
        emit signal_req_control(CT_RAIN,-1);
    }
    else
    {
        cur_text.replace("Auto","Manual");
        ui->horizontalSliderRain->setEnabled(true);
        emit signal_req_control(CT_RAIN,10);
    }
    ui->pushButtonRain->setText(cur_text);
}

void FrameLeft::on_horizontalSliderRain_valueChanged(int value)
{
    emit signal_req_control(CT_RAIN,value);
}

void FrameLeft::on_checkBoxMTI_clicked(bool checked)
{
    mti_settings.enable = checked;
}

void FrameLeft::on_horizontalSliderMTI_valueChanged(int value)
{
    mti_settings.threshold = (quint8)value;
    ui->lineEditMTI->setText(QString::number(value));
}

void FrameLeft::resizeEvent(QResizeEvent *event)
{
    qDebug()<<Q_FUNC_INFO<<event->size()<<qApp->desktop()->size();

    /*
    int margin = height() - ui->groupBoxSubSistemStatus->height();
    if(margin >= qApp->desktop()->size().height())
    {
        ui->groupBoxSubSistemStatus->hide();
        resize(width(),qApp->desktop()->size().height());
        qDebug()<<Q_FUNC_INFO<<event->size()<<qApp->desktop()->size();
    }
    */
}
