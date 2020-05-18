#ifndef STREAMDEVICE_H
#define STREAMDEVICE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>

//#include "global.h"
//#include "qextserialport.h"

enum StreamMode
{
    In = 0,
    Out,
    Idle
};

enum StreamType
{
    Serial = 0,
    TCP_CLIENT,
    UDP,
    TCP_SERVER
};

struct StreamSettings
{
    StreamMode mode;
    StreamType type;
    QString config;
};

enum SensorStatus
{
    NOT_AVAIL = 0,
    NO_DATA,
    DATA_UNKNOWN,
    AVAIL
};

class StreamDevice : public QObject
{
    Q_OBJECT
public:
    explicit StreamDevice(QObject *parent = 0, StreamSettings settings = StreamSettings());
    
    QByteArray readData();
    void sendData(QByteArray data);
    QString getCurrentError() { return m_error; }
    bool isTcpConnected()
    {
        m_error = tcpsocket.state()==QAbstractSocket::ConnectedState ? "" : m_error;
        return tcpsocket.state()==QAbstractSocket::ConnectedState;
    }

signals:
    
private slots:
    
private:
//    QextSerialPort *serialPort;
    QTcpSocket tcpsocket;
    QUdpSocket udpsocket;
    StreamSettings m_settings;
    QString m_error;

    void init();
    QString setTcpClient();
    QString setUdp();

    /*
    QString setSerial();
    BaudRateType setBaud(QString stringBaud);
    DataBitsType setDataBits(QString stringDataBits);
    ParityType setParity(QString stringParity);
    StopBitsType setStopBit(QString stringStopBit);
    FlowType setFlowControl(QString stringFlowControl);
    */
};

#endif // STREAMDEVICE_H
