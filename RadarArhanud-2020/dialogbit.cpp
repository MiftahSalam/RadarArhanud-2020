#include "dialogbit.h"
#include "ui_dialogbit.h"
#include "radarengine_global.h"

#include <QSettings>
#include <QTcpSocket>
#include <QHostAddress>

DialogBIT::DialogBIT(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBIT)
{
    ui->setupUi(this);
}

DialogBIT::~DialogBIT()
{
    delete ui;
}

void DialogBIT::on_pushButtonBITRadar_clicked()
{
    if(state_radar == RADAR_OFF)
    {
        ui->labelRadarPowerTest->setText("Fail");
        ui->labelRadarPowerTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelRadarConnTest->setText("Fail");
        ui->labelRadarConnTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelRadarOpTest->setText("Fail");
        ui->labelRadarOpTest->setStyleSheet("background-color: rgb(164,0,0);");
    }
    else if(state_radar == RADAR_WAKING_UP)
    {
        ui->labelRadarPowerTest->setText("OK");
        ui->labelRadarPowerTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelRadarConnTest->setText("OK");
        ui->labelRadarConnTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelRadarOpTest->setText("OK (Waking up)");
        ui->labelRadarOpTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else if(state_radar == RADAR_STANDBY)
    {
        ui->labelRadarPowerTest->setText("OK");
        ui->labelRadarPowerTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelRadarConnTest->setText("OK");
        ui->labelRadarConnTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelRadarOpTest->setText("OK (Standby)");
        ui->labelRadarOpTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else if(state_radar == RADAR_TRANSMIT)
    {
        ui->labelRadarPowerTest->setText("OK");
        ui->labelRadarPowerTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelRadarConnTest->setText("OK");
        ui->labelRadarConnTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelRadarOpTest->setText("OK (Transmit)");
        ui->labelRadarOpTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
}

void DialogBIT::on_pushButtonBITAnthenaSW_clicked()
{
    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");
    QString ip = config.value("antene_switch/ip","127.0.0.1").toString();
    QString port = config.value("antene_switch/port","80").toString();
    QTcpSocket socket;

    qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    socket.connectToHost(QHostAddress(ip),port.toInt(),QIODevice::ReadOnly);
    if(socket.waitForConnected(3000))
    {
        ui->labelAnthSWPowerTest->setText("OK");
        ui->labelAnthSWPowerTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelAnthSWConTest->setText("OK");
        ui->labelAnthSWConTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelAnthSWOpTest->setText("OK");
        ui->labelAnthSWOpTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelAnthSWPowerTest->setText("Fail");
        ui->labelAnthSWPowerTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelAnthSWConTest->setText("Fail");
        ui->labelAnthSWConTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelAnthSWOpTest->setText("Fail");
        ui->labelAnthSWOpTest->setStyleSheet("background-color: rgb(164,0,0);");
    }
    qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void DialogBIT::on_pushButtonBITTilting_clicked()
{
    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");
    QString ip = config.value("tilting/ip","127.0.0.1").toString();
    QString port = config.value("tilting/port","80").toString();
    QTcpSocket socket;

    qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    socket.connectToHost(QHostAddress(ip),port.toInt(),QIODevice::ReadOnly);
    if(socket.waitForConnected(3000))
    {
        ui->labelTiltingPowerTest->setText("OK");
        ui->labelTiltingPowerTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelTiltingConTest->setText("OK");
        ui->labelTiltingConTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelTiltingOpTest->setText("OK (Online)");
        ui->labelTiltingOpTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelTiltingPowerTest->setText("Fail");
        ui->labelTiltingPowerTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelTiltingConTest->setText("Fail");
        ui->labelTiltingConTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelTiltingOpTest->setText("Fail");
        ui->labelTiltingOpTest->setStyleSheet("background-color: rgb(164,0,0);");
    }
    qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));

}

void DialogBIT::on_pushButtonBITLora_clicked()
{
    //ip = 11
    QString ip = "192.168.1.11";
    QString port = "80";
    QTcpSocket socket;

    qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    socket.connectToHost(QHostAddress(ip),port.toInt(),QIODevice::ReadOnly);
    if(socket.waitForConnected(3000))
    {
        ui->labelLoraPowerTest->setText("OK");
        ui->labelLoraPowerTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelLoraConTest->setText("OK");
        ui->labelLoraConTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelLoraOpTest->setText("OK (Online)");
        ui->labelLoraOpTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelLoraPowerTest->setText("Fail");
        ui->labelLoraPowerTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelLoraConTest->setText("Fail");
        ui->labelLoraConTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelLoraOpTest->setText("Fail");
        ui->labelLoraOpTest->setStyleSheet("background-color: rgb(164,0,0);");
    }
    qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));

}

void DialogBIT::on_pushButtonBITNav_clicked()
{
    gps_status.gps_online = true; //tes sementara fix dulu
    gps_status.gps_valid = true; //tes sementara fix dulu

    if(gps_status.gps_online)
    {
        ui->labelNavPowerTest->setText("OK");
        ui->labelNavPowerTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelNavConTest->setText("OK");
        ui->labelNavConTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelNavOpTest->setText("OK (Online)");
        ui->labelNavOpTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelNavPowerTest->setText("Fail");
        ui->labelNavPowerTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelNavConTest->setText("Fail");
        ui->labelNavConTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelNavOpTest->setText("Fail");
        ui->labelNavOpTest->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(gps_status.gps_valid)
    {
        ui->labelNavGPSValidTest->setText("OK");
        ui->labelNavGPSValidTest->setStyleSheet("background-color: rgb(78, 154, 6);");
        ui->labelNavHdtValidTest->setText("OK");
        ui->labelNavHdtValidTest->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelNavGPSValidTest->setText("Fail");
        ui->labelNavGPSValidTest->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelNavHdtValidTest->setText("Fail");
        ui->labelNavHdtValidTest->setStyleSheet("background-color: rgb(164,0,0);");
    }
}
