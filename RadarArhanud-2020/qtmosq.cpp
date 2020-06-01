#include "qtmosq.h"
#include <QSettings>
#include <QDir>
#include <QDebug>

void qtmosq::on_connect(int result)
{
    qDebug()<<Q_FUNC_INFO<<result;
    _connected = true;
    if (!result)
    {
        //subscribe(NULL, "$SYS/#", 2);
        emit connected();
    }
    else
        emit connectEnable();
}
void qtmosq::on_disconnect(int result)
{
    qDebug()<<Q_FUNC_INFO<<result;
    _connected = false;
    if (!result)
    {
        //subscribe(NULL, "$SYS/#", 2);
        emit disconnected();
    }
    else
        emit disconnectEnable();
}
void qtmosq::on_publish(int id)
{
    MID++;
    emit messageSent(true);
}
void qtmosq::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    if (MID != 0)
        emit subscribed();
    MID++;
}
void qtmosq::on_message(const mosquitto_message *message)
{
    if (message->payloadlen)
    {
        QString s(message->topic);
        s += '>';
        s += (char*) message->payload;
        emit messageReceived(s);
    }
}

int* qtmosq::getMID() {return &MID;}

bool qtmosq::isConnected(){return _connected;}


qtmosq *mqtt = 0;
qtmosq *getMQTT()
{
    if(!mqtt)
    {
        QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");

        QString id = config.value("mqtt/id",false).toString();
        QString ip = config.value("mqtt/ip",false).toString();
        uint port = config.value("mqtt/port",false).toUInt();

        lib_init();
        mqtt = new qtmosq(id.toUtf8().constData(), false);
        int con_result = mqtt->connect_async(ip.toUtf8().constData(), port);
        mqtt->loop_start();

        qDebug()<<Q_FUNC_INFO<<id<<ip<<port<<con_result;
    }
    return mqtt;
}
