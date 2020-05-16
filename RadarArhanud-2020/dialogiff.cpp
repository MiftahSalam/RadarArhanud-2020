#include "dialogiff.h"
#include "ui_dialogiff.h"
#include "echo/radar_global.h"

DialogIFF::DialogIFF(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogIFF)
{
    ui->setupUi(this);

    ui->lineEditIPData->setValidator(new QIntValidator(0,255,ui->lineEditIPData));
    ui->lineEditPortData->setValidator(new QIntValidator(3000,65536,ui->lineEditPortData));

    qDebug()<<Q_FUNC_INFO<<iff_settings.ip;
    ui->lineEditIPData->setText(iff_settings.ip);
    ui->lineEditPortData->setText(QString::number(iff_settings.port));

}

DialogIFF::~DialogIFF()
{
    delete ui;
}

void DialogIFF::on_pushButtonApply_clicked()
{
    iff_settings.ip = ui->lineEditIPData->text().remove(" ");
    iff_settings.port = ui->lineEditPortData->text().toUInt();

    emit signal_settingChange();
}
