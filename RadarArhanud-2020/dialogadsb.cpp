#include "dialogadsb.h"
#include "ui_dialogadsb.h"
#include <radarengine_global.h>
#include <stream-arhnd/stream/streamdevice.h>

#include <QIntValidator>

DialogADSB::DialogADSB(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogADSB)
{
    ui->setupUi(this);

//    ui->lineEditIPData->setValidator(new QIntValidator(0,255,ui->lineEditIPData));
    ui->lineEditPortData->setValidator(new QIntValidator(3000,65536,ui->lineEditPortData));
    ui->checkBoxShowTrack->setChecked(adsb_settings.show_track);
    ui->checkBoxShowTrackData->setChecked(adsb_settings.show_attr);
    QStringList conf_list = adsb_settings.config.split(";");
    qDebug()<<Q_FUNC_INFO<<conf_list;

    if(conf_list.size() == 2)
    {
        ui->lineEditIPData->setText(conf_list.at(0));
        ui->lineEditPortData->setText(conf_list.at(1));
    }

}

void DialogADSB::setStatus(int status)
{
    switch (status)
    {
    case 0:
        ui->labelConnection->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelConnection->setText("Offline");
        ui->labelError->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelError->setText("Not available");
        break;
    case 1:
        ui->labelConnection->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelConnection->setText("Online");
        ui->labelError->setStyleSheet("background-color: rgb(196, 160, 0);");
        ui->labelError->setText("No Data");
        break;
    case 2:
        ui->labelConnection->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelConnection->setText("Online");
        ui->labelError->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelError->setText("Data Unknown");
        break;
    case 3:
        ui->labelConnection->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelConnection->setText("Online");
        ui->labelError->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelError->setText("No Error");
        break;
    default:
        break;
    }
}

DialogADSB::~DialogADSB()
{
    delete ui;
}

void DialogADSB::on_pushButtonApply_clicked()
{
    QStringList conf_list;

    conf_list<<ui->lineEditIPData->text().remove(" ")<<ui->lineEditPortData->text();
    adsb_settings.config = conf_list.join(";");
    adsb_settings.type = StreamArhnd::HTTP;

    emit signal_settingChange();
}

void DialogADSB::on_checkBoxShowTrack_clicked(bool checked)
{
    adsb_settings.show_track = checked;
    adsb_settings.type = StreamArhnd::HTTP;

    emit signal_settingChange();
}

void DialogADSB::on_checkBoxShowTrackData_clicked(bool checked)
{
    adsb_settings.show_attr = checked;
    adsb_settings.type = StreamArhnd::HTTP;

    emit signal_settingChange();
}
