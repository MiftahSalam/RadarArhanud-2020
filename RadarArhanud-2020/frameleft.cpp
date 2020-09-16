#include "frameleft.h"
#include "ui_frameleft.h"
#include <radarengine.h>

#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QDesktopWidget>
#include <QMessageBox>

FrameLeft::FrameLeft(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameLeft)
{
    ui->setupUi(this);

    QIntValidator *int_validator = new QIntValidator(0,255,this);

    ui->lineEditGain->setValidator(int_validator);
    ui->lineEditRain->setValidator(int_validator);

    ui->checkBoxShowRing->setChecked(radar_settings.show_rings);
    ui->checkBoxShowMap->setChecked(map_settings.show);
    ui->comboBoxMapMode->setCurrentIndex((int)map_settings.mode);
    ui->checkBoxShowHM->setChecked(radar_settings.show_heading_marker);
    ui->checkBoxShowCompass->setChecked(radar_settings.show_compass);
    ui->checkBoxMTI->setChecked(mti_settings.enable);
    ui->horizontalSliderMTI->setValue(mti_settings.threshold);
    ui->lineEditMTI->setText(QString::number(mti_settings.threshold));

    if(qApp->desktop()->height() < 1000)
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

//    ui->labelRange->setText("64 NM");
    cur_zoom_lvl = 10;
    cur_antene = 1;
    first_switch = 0;

    connect(dRadar,SIGNAL(signal_settingChange()),this,SIGNAL(signal_radarSettingChange()));
    connect(dADSB,SIGNAL(signal_settingChange()),this,SIGNAL(signal_adsbSettingChange()));
    connect(dTrail,&TrailDialog::signal_clearTrailReq,this,&FrameLeft::signal_clearTrail);
//    state_radar = RADAR_STANDBY; //temporary for test
//    trigger_stateChange(); //temporary for test
}
void FrameLeft::setAdsbStatus(int status)
{
    dADSB->setStatus(status);
}

void FrameLeft::setRangeRings(qreal range)
{
    ui->labelRingRange->setText(QString::number(range,'f',2)+" Km");
}

void FrameLeft::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    qDebug()<<Q_FUNC_INFO;
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
        emit signal_req_range();
        emit signal_Tx();
    }
    else if(ui->pushButtonTxStnb->text() == "Standby")
        emit signal_Standby();
}

void FrameLeft::trigger_stateChange()
{
    qDebug()<<Q_FUNC_INFO<<(int)state_radar;

//    state_radar = RADAR_STANDBY; //faking
    if((state_radar == RADAR_OFF) || (state_radar == RADAR_WAKING_UP))
    {
        cur_antene = 1;
        first_switch = 0;
        ui->pushButtonTxStnb->setText("Tx/Stby");
        ui->pushButtonGain->setEnabled(false);
        ui->pushButtonRain->setEnabled(false);
        ui->horizontalSliderRain->setEnabled(false);
        ui->horizontalSliderGain->setEnabled(false);
        ui->pushButtonTxStnb->setEnabled(false);
    }
    else if(state_radar == RADAR_STANDBY)
    {
        cur_antene = 1;
        first_switch = 0;
        ui->pushButtonTxStnb->setText("Transmit");
        ui->pushButtonGain->setEnabled(true);
        ui->pushButtonRain->setEnabled(true);
        ui->horizontalSliderRain->setEnabled(true);
        ui->horizontalSliderGain->setEnabled(true);
        ui->pushButtonTxStnb->setEnabled(true);
    }
    else if(state_radar == RADAR_TRANSMIT|| (state_radar == RADAR_NO_SPOKE))
    {
        ui->pushButtonTxStnb->setText("Standby");
        ui->pushButtonGain->setEnabled(true);
        ui->pushButtonRain->setEnabled(true);
        ui->horizontalSliderRain->setEnabled(true);
        ui->horizontalSliderGain->setEnabled(true);
        ui->pushButtonTxStnb->setEnabled(true);
    }

    if(socket.state() != QAbstractSocket::ConnectedState)
        socket.connectToHost(antene_switch_settings.ip,antene_switch_settings.port);
}

void FrameLeft::trigger_changeAntene()
{
    if(first_switch)
    {
        cur_antene++;
        if(cur_antene>2)
            cur_antene = 0;
        socket.write(QString::number(cur_antene+1).toUtf8());
    }
    else
        first_switch = 1;

    qDebug()<<Q_FUNC_INFO<<cur_antene<<first_switch;

}

void FrameLeft::trigger_reportChange()
{
//    qDebug()<<Q_FUNC_INFO<<filter.gain<<filter.rain<<filter.sea;
//    ui->horizontalSliderGain->setValue(filter.gain);
//    ui->horizontalSliderRain->setValue(filter.rain);
//    ui->lineEditGain->setText(QString::number(filter.gain));
//    ui->lineEditRain->setText(QString::number(filter.rain));
}

void FrameLeft::on_pushButtonZoomIn_clicked()
{
    cur_zoom_lvl++;
    if(cur_zoom_lvl > distanceList.size()-1)
        cur_zoom_lvl = distanceList.size()-1;

    /*
    qDebug()<<Q_FUNC_INFO<<ui->labelRange->text();

    int g;
    QString rngName = ui->labelRange->text();
    for (g = ARRAY_SIZE(g_ranges_metric)-1; g > 0; g--)
    {
        if (QString(g_ranges_metric[g].name )== rngName)
            break;
    }
    g--;
    if(g < 0)
        g = 0;

    ui->labelRange->setText(g_ranges_metric[g].name);
    qDebug()<<Q_FUNC_INFO<<g<<g_ranges_metric[g].name;
    */
    emit signal_req_range();
}

void FrameLeft::setRangeText(double range,bool match)
{
    ui->labelRange->setText(QString::number(range,'f',1)+" Km");
    if(match)
        ui->labelRange->setStyleSheet(QStringLiteral("color: rgb(255, 255, 0);"));
    else
        ui->labelRange->setStyleSheet(QStringLiteral("color: rgb(255, 0, 0);"));
}

void FrameLeft::on_pushButtonZoomOut_clicked()
{
    cur_zoom_lvl--;
    if(cur_zoom_lvl < 8)
        cur_zoom_lvl = 8;

    /*
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
    */
    emit signal_req_range();
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
    ui->lineEditGain->setText(QString::number(value));
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
    ui->lineEditRain->setText(QString::number(value));
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
}

void FrameLeft::on_pushButtonARPA_clicked()
{
    system("gedit"); //path to tilt application
}

void FrameLeft::on_pushButtonShutdown_clicked()
{
    if(QMessageBox::information(this,"Information","Really quit application?",
                                QMessageBox::Ok,
                                QMessageBox::No) == QMessageBox::Ok)
        emit signal_exit();
}

void FrameLeft::on_lineEditGain_editingFinished()
{
    ui->horizontalSliderGain->setValue(ui->lineEditGain->text().toInt());
}

void FrameLeft::on_lineEditRain_editingFinished()
{
    ui->horizontalSliderRain->setValue(ui->lineEditRain->text().toInt());
}
