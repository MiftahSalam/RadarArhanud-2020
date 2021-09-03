#include "dialogiff.h"
#include "ui_dialogiff.h"
#include <radarengine_global.h>

#include <QIntValidator>

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

    //test
    StreamArhnd::StreamSettings iffSettingIn, iffSettingOut;
    iffSettingIn.config = "127.0.0.1;8090";
    iffSettingIn.mode = StreamArhnd::In;
    iffSettingIn.type = (StreamArhnd::StreamType)2; //udp
    iffSettingOut.config = "127.0.0.1;8070";
    iffSettingOut.mode = StreamArhnd::Out;
    iffSettingOut.type = (StreamArhnd::StreamType)1; //tcp

    iff = new IFFArhnd::IFFService(this, iffSettingIn, iffSettingOut);
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
