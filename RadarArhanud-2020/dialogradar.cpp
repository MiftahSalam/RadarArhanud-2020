#include "dialogradar.h"
#include "ui_dialogradar.h"
#include "echo/radar_global.h"

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

    qDebug()<<Q_FUNC_INFO<<radar_settings.ip_data;
    ui->lineEditIPData->setText(radar_settings.ip_data);
    ui->lineEditPortData->setText(QString::number(radar_settings.port_data));
    ui->lineEditIPReport->setText(radar_settings.ip_report);
    ui->lineEditPortReport->setText(QString::number(radar_settings.port_report));
    ui->lineEditIPcmd->setText(radar_settings.ip_command);
    ui->lineEditPortcmd->setText(QString::number(radar_settings.port_command));

}

DialogRadar::~DialogRadar()
{
    delete ui;
}

void DialogRadar::on_pushButtonApply_clicked()
{
    radar_settings.ip_data = ui->lineEditIPData->text().remove(" ");
    radar_settings.port_data = ui->lineEditPortData->text().toUInt();
    radar_settings.ip_report = ui->lineEditIPReport->text().remove(" ");
    radar_settings.port_report = ui->lineEditPortReport->text().toUInt();
    radar_settings.ip_command = ui->lineEditIPcmd->text().remove(" ");
    radar_settings.port_command = ui->lineEditPortcmd->text().toUInt();

    emit signal_settingChange();
}
