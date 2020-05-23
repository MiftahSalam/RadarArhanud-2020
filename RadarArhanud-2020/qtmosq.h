#ifndef QTMOSQ_H
#define QTMOSQ_H

#include <QObject>

#include <mosquittopp.h>
#include <mosquitto.h>

using namespace mosqpp;

class qtmosq : public QObject, public mosquittopp
{
    Q_OBJECT

public:
    qtmosq(const char *id = NULL, bool clean_session = true) : mosquittopp (id, clean_session) {MID = 0;_connected = false;}
    ~qtmosq() {}

    void on_connect(int result);
    void on_disconnect(int result);
    void on_publish(int id);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
    void on_message(const mosquitto_message *message);

    int* getMID();

    bool isConnected();

private:
    int MID;
    bool _connected;

signals:
    void connected();
    void messageSent(bool);
    void messageReceived(QString);
    void connectEnable();
    void subscribed();
    void disconnectEnable();
    void disconnected();

};

extern qtmosq *mqtt;
qtmosq *getMQTT();

#endif // QTMOSQ_H
