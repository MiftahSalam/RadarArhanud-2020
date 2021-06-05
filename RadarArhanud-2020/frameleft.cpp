#include "frameleft.h"
#include "ui_frameleft.h"
#include <radarengine.h>

#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>

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
    {
        ui->groupBoxSubSistemStatus->hide();
//        ui->groupBoxSubRoomStatus->hide();
    }

    dRadar = new DialogRadar(this);
    dIFF = new DialogIFF(this);
    dADSB = new DialogADSB(this);
    dTrail = new TrailDialog(this);
    dLog = new DialogLogging(this);

//    dRadar->setModal(true);
//    dIFF->setModal(true);
//    dADSB->setModal(true);
//    dTrail->setModal(true);
//    dLog->setModal(true);

    cur_zoom_lvl = 10;
    antena_switch = 1;
    first_switch = 0;

    if(radar_settings.op_mode)
    {
        ui->pushButtonZoomOut->setEnabled(false);
        ui->pushButtonZoomIn->setEnabled(false);
    }

    connect(dRadar,SIGNAL(signal_settingChange()),this,SIGNAL(signal_radarSettingChange()));
    connect(dADSB,SIGNAL(signal_settingChange()),this,SIGNAL(signal_adsbSettingChange()));
    connect(dTrail,&TrailDialog::signal_clearTrailReq,this,&FrameLeft::signal_clearTrail);
//    state_radar = RADAR_STANDBY; //temporary for test
//    trigger_stateChange(); //temporary for test

    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");

    qDebug()<<Q_FUNC_INFO<<QProcess::startDetached("killall \"/usr/bin/python3\"");
    qDebug()<<Q_FUNC_INFO<<QProcess::startDetached(config.value("nav_sensor/cmd","python3 /home/miftah/serial2mqtt.py").toString());

}

void FrameLeft::setNavStatus(int status)
{
    qDebug()<<Q_FUNC_INFO<<status;

    switch (status)
    {
//    case 0:
    case 1:
        ui->labelNavStatus->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelNavStatus->setText("Online");
        break;
//    case 1:
    case 0:
        ui->labelNavStatus->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelNavStatus->setText("Offline");
        break;
    default:
        break;
    }

}

void FrameLeft::updateRadarStatus()
{
    switch (state_radar)
    {
    case RADAR_TRANSMIT:
        ui->labelRadarStatus->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelRadarStatus->setText("Trasmitting");
        break;
    case RADAR_OFF:
        ui->labelRadarStatus->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelRadarStatus->setText("No Radar");
        break;
    case RADAR_STANDBY:
        ui->labelRadarStatus->setStyleSheet("background-color: rgb(196, 160, 0);");
        ui->labelRadarStatus->setText("Standby");
        break;
    case RADAR_WAKING_UP:
        ui->labelRadarStatus->setStyleSheet("background-color: rgb(196, 160, 0);");
        ui->labelRadarStatus->setText("Warming up");
        break;
    default:
        break;
    }

}

void FrameLeft::setAdsbStatus(int status)
{
    dADSB->setStatus(status);

    switch (status)
    {
    case 0:
        ui->labelAdsbStatus->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelAdsbStatus->setText("Offline");
        break;
    case 1:
        ui->labelAdsbStatus->setStyleSheet("background-color: rgb(196, 160, 0);");
        ui->labelAdsbStatus->setText("No Data");
        break;
    case 2:
        ui->labelAdsbStatus->setStyleSheet("background-color: rgb(196, 160, 0);");
        ui->labelAdsbStatus->setText("Data Unknown");
        break;
    case 3:
        ui->labelAdsbStatus->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelAdsbStatus->setText("Online");
        break;
    default:
        break;
    }

}

void FrameLeft::setRangeRings(qreal range)
{
    ui->labelRingRange->setText(QString::number(range,'f',2)+" Km");
}

void FrameLeft::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    qDebug()<<Q_FUNC_INFO;

    QMenu menu;
    QAction act("Fix Range Mode",this);
    act.setCheckable(true);
    act.setChecked(radar_settings.op_mode);
    connect(&act,SIGNAL(triggered(bool)),this,SLOT(trigger_changeOpMode(bool)));

    menu.addAction(&act);
    menu.exec(event->pos());
}

void FrameLeft::trigger_changeOpMode(bool checked)
{
    qDebug()<<Q_FUNC_INFO<<checked;

    ui->pushButtonZoomOut->setEnabled(!checked);
    ui->pushButtonZoomIn->setEnabled(!checked);
    dRadar->trigger_fixRangeMode(checked);

    if(checked) dRadar->resize(dRadar->width(),dRadar->height()/2);
    else dRadar->resize(dRadar->width(),dRadar->height());

    emit signal_changeOpMode(checked);
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
        antena_switch = 1;
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
        antena_switch = 1;
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
        antena_switch++;
        if(antena_switch>2)
            antena_switch = 0;
        socket.write(QString::number(antena_switch+1).toUtf8());
    }
    else
        first_switch = 1;

    qDebug()<<Q_FUNC_INFO<<antena_switch<<first_switch;

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

    emit signal_req_range();
}

void FrameLeft::setRangeText(double range,bool match)
{
    qDebug()<<Q_FUNC_INFO<<"range"<<range<<"match"<<match;
    if(range > 1)
        ui->labelRange->setText(QString::number((int)range)+" Km");
    else
        ui->labelRange->setText(QString::number((int)(range*1000.))+" m");

    ui->labelRange->setStyleSheet(QStringLiteral("color: rgb(255, 255, 0);"));
    if(!match)
    {
        if(range < 50.)
            ui->labelRange->setStyleSheet(QStringLiteral("color: rgb(255, 0, 0);"));
    }
}

void FrameLeft::on_pushButtonZoomOut_clicked()
{
    cur_zoom_lvl--;
    if(cur_zoom_lvl < 8)
        cur_zoom_lvl = 8;

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
    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");
    qDebug()<<Q_FUNC_INFO<<QProcess::startDetached(config.value("tilting/path","/home/arhanud/aktuator").toString());

//    system(config.value("tilting/path","/home/arhanud/aktuator").toString().toUtf8()); //path to tilt application
}

void FrameLeft::on_pushButtonShutdown_clicked()
{
    if(state_radar == RADAR_TRANSMIT)
    {
        QMessageBox::information(this,"Information","Radar is transmitting.\nPlease switch to standby mode first before quitting",
                                 QMessageBox::Ok);
        return;
    }

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

void FrameLeft::on_lineEditMTI_editingFinished()
{
    ui->horizontalSliderMTI->setValue(ui->lineEditMTI->text().toInt());
}
