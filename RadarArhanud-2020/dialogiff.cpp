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
//    iffSettingOut.config = "127.0.0.1;8070";
//    iffSettingOut.mode = StreamArhnd::InOut;
//    iffSettingOut.type = (StreamArhnd::StreamType)1; //tcp
    iffSettingOut.config = "/dev/ttyUSB0;230400";
    iffSettingOut.mode = StreamArhnd::InOut;
    iffSettingOut.type = (StreamArhnd::StreamType)0; //serial

    iff = new IFFArhnd::IFFService(this, iffSettingIn, iffSettingOut);
    iff->setLatLonHdt(currentOwnShipLat, currentOwnShipLon, currentHeading); //tes
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

void DialogIFF::on_pushButtonApplyFlightID_clicked()
{
    if(ui->lineEditFlightIDICAO->text().isEmpty())
        iff->setFlightID(ui->lineEditFlightIDICAO->text());
}

void DialogIFF::on_pushButtonApplyOperating_clicked()
{
    QString mode, es;

    switch (ui->comboBoxOpMode->currentIndex()) {
    case 0:
        mode = "STANDBY";
        break;
    case 1:
        mode = "ON";
        break;
    case 2:
        mode = "ALT";
        break;
    default:
        break;
    }

    if(ui->checkBoxOpEnableES->isChecked()) es = "ES_EN";
    else es = "ES_DIS";

    QStringList config = QStringList()<<ui->lineEditOpSquawk->text()
                                     <<mode+","+"PWR_STATE_DIS"+","+es
                                    <<"0"
                                   <<"0"
                                  <<"0"
                                 <<"0"
                                <<QString::number(currentHeading)
                                  ;
    iff->setOpMode(config.join(";"));
}

void DialogIFF::on_pushButtonApplyTrgReqSend_clicked()
{
    int report_req = 0;

    if(ui->checkBoxTrgReqPosSend) report_req += 0x01;
    if(ui->checkBoxTrgReqPosSend) report_req += 0x80;

    QStringList config = QStringList()<<QString::number(ui->comboBoxTrgReqMode->currentIndex())
                                     <<QString::number(ui->comboBoxTrgReqTxPort->currentIndex())
                                    <<ui->spinBoxTrgReqParticipans->text()
                                   <<"03FE14"
                                  <<QString::number(report_req)
                                    ;
    iff->trackReq(config.join(";"));
}
