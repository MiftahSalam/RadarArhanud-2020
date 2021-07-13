#include "dialogradar.h"
#include "ui_dialogradar.h"
#include <RadarEngine/radarengine_global.h>

#include <QIntValidator>

DialogRadar::DialogRadar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRadar)
{
    ui->setupUi(this);

    ui->lineEditIPData->setValidator(new QIntValidator(0,255,ui->lineEditIPData));
    ui->lineEditPortData->setValidator(new QIntValidator(3000,65536,ui->lineEditPortData));
    ui->lineEditIPReport->setValidator(new QIntValidator(0,255,ui->lineEditIPReport));
    ui->lineEditPortReport->setValidator(new QIntValidator(3000,65536,ui->lineEditPortReport));
    ui->lineEditIPcmd->setValidator(new QIntValidator(0,255,ui->lineEditIPcmd));
    ui->lineEditPortcmd->setValidator(new QIntValidator(3000,65536,ui->lineEditPortcmd));

    ui->checkBoxShowTrackData->setChecked(arpa_settings[0].show_attr);
    ui->checkBoxShowEnable->setChecked(radar_settings.enable);
    ui->lineEditIPData->setText(radar_settings.ip_data);
    ui->lineEditPortData->setText(QString::number(radar_settings.port_data));
    ui->lineEditIPReport->setText(radar_settings.ip_report);
    ui->lineEditPortReport->setText(QString::number(radar_settings.port_report));
    ui->lineEditIPcmd->setText(radar_settings.ip_command);
    ui->lineEditPortcmd->setText(QString::number(radar_settings.port_command));

    ui->lineEditIPData_2->setValidator(new QIntValidator(0,255,ui->lineEditIPData));
    ui->lineEditPortData_2->setValidator(new QIntValidator(3000,65536,ui->lineEditPortData));
    ui->lineEditIPReport_2->setValidator(new QIntValidator(0,255,ui->lineEditIPReport));
    ui->lineEditPortReport_2->setValidator(new QIntValidator(3000,65536,ui->lineEditPortReport));
    ui->lineEditIPcmd_2->setValidator(new QIntValidator(0,255,ui->lineEditIPcmd));
    ui->lineEditPortcmd_2->setValidator(new QIntValidator(3000,65536,ui->lineEditPortcmd));

    ui->checkBoxShowTrackData_2->setChecked(arpa_settings[0].show_attr);
    ui->checkBoxShowEnable_2->setChecked(radar_settings.enable1);
    ui->lineEditIPData_2->setText(radar_settings.ip_data1);
    ui->lineEditPortData_2->setText(QString::number(radar_settings.port_data1));
    ui->lineEditIPReport_2->setText(radar_settings.ip_report1);
    ui->lineEditPortReport_2->setText(QString::number(radar_settings.port_report1));
    ui->lineEditIPcmd_2->setText(radar_settings.ip_command1);
    ui->lineEditPortcmd_2->setText(QString::number(radar_settings.port_command1));

    fullHeight = height();
    halfHeight = height()/2;

    qDebug()<<Q_FUNC_INFO<<"fullHeight size:"<<fullHeight<<"halfHeight"<<halfHeight;

    if(radar_settings.op_mode) {
        ui->groupBox_2->hide();
        resize(width(),halfHeight);
    }
}

void DialogRadar::trigger_fixRangeMode(bool fix)
{
    qDebug()<<Q_FUNC_INFO<<"fix"<<fix<<"fullHeight size:"<<fullHeight<<"halfHeight"<<halfHeight;
    if(fix) {
        ui->groupBox_2->hide();
        resize(width(),halfHeight);
    } else {
        ui->groupBox_2->show();
        resize(width(),fullHeight);
    }
}

void DialogRadar::resizeEvent(QResizeEvent *evt)
{
    qDebug()<<Q_FUNC_INFO<<"new size:"<<evt->size();
}

DialogRadar::~DialogRadar()
{
    delete ui;
}

void DialogRadar::on_pushButtonApply_clicked()
{
    radar_settings.enable = ui->checkBoxShowEnable->isChecked();
    radar_settings.ip_data = ui->lineEditIPData->text().remove(" ");
    radar_settings.port_data = ui->lineEditPortData->text().toUInt();
    radar_settings.ip_report = ui->lineEditIPReport->text().remove(" ");
    radar_settings.port_report = ui->lineEditPortReport->text().toUInt();
    radar_settings.ip_command = ui->lineEditIPcmd->text().remove(" ");
    radar_settings.port_command = ui->lineEditPortcmd->text().toUInt();
    arpa_settings[0].show_attr = ui->checkBoxShowTrackData->isChecked();

    emit signal_settingChange();
}

void DialogRadar::on_checkBoxShowTrackData_clicked(bool checked)
{
    arpa_settings[0].show_attr = checked;
}

void DialogRadar::on_pushButtonApply_2_clicked()
{
    radar_settings.enable1 = ui->checkBoxShowEnable_2->isChecked();
    radar_settings.ip_data1 = ui->lineEditIPData_2->text().remove(" ");
    radar_settings.port_data1 = ui->lineEditPortData_2->text().toUInt();
    radar_settings.ip_report1 = ui->lineEditIPReport_2->text().remove(" ");
    radar_settings.port_report1 = ui->lineEditPortReport_2->text().toUInt();
    radar_settings.ip_command1 = ui->lineEditIPcmd_2->text().remove(" ");
    radar_settings.port_command1 = ui->lineEditPortcmd_2->text().toUInt();
    arpa_settings[1].show_attr = ui->checkBoxShowTrackData_2->isChecked();

    state_radar1 = RADAR_STANDBY;

    emit signal_settingChange();
}
