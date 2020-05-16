#include "dialogadsb.h"
#include "ui_dialogadsb.h"
#include "echo/radar_global.h"

DialogADSB::DialogADSB(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogADSB)
{
    ui->setupUi(this);

    ui->lineEditIPData->setValidator(new QIntValidator(0,255,ui->lineEditIPData));
    ui->lineEditPortData->setValidator(new QIntValidator(3000,65536,ui->lineEditPortData));

    qDebug()<<Q_FUNC_INFO<<adsb_settings.ip;
    ui->lineEditIPData->setText(adsb_settings.ip);
    ui->lineEditPortData->setText(QString::number(adsb_settings.port));

}

DialogADSB::~DialogADSB()
{
    delete ui;
}

void DialogADSB::on_pushButtonApply_clicked()
{
    adsb_settings.ip = ui->lineEditIPData->text().remove(" ");
    adsb_settings.port = ui->lineEditPortData->text().toUInt();

    emit signal_settingChange();
}
