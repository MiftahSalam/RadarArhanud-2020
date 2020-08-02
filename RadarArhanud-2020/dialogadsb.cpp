#include "dialogadsb.h"
#include "ui_dialogadsb.h"
#include <radarengine_global.h>

DialogADSB::DialogADSB(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogADSB)
{
    ui->setupUi(this);

    ui->lineEditIPData->setValidator(new QIntValidator(0,255,ui->lineEditIPData));
    ui->lineEditPortData->setValidator(new QIntValidator(3000,65536,ui->lineEditPortData));

    QStringList conf_list = adsb_settings.config.split(";");
    qDebug()<<Q_FUNC_INFO<<conf_list;

    if(conf_list.size() == 2)
    {
        ui->lineEditIPData->setText(conf_list.at(0));
        ui->lineEditPortData->setText(conf_list.at(1));
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

    emit signal_settingChange();
}
