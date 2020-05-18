#ifndef STREAM_H
#define STREAM_H

#include <QObject>
#include <QThread>
#include <QMutex>

#include "streamdevice.h"

class Stream : public QThread
{
    Q_OBJECT
public:
    explicit Stream(QThread *parent = 0, StreamSettings settings = StreamSettings());

    ~Stream()
    {
        mutex.lock();
        exitReq = true;
        mutex.unlock();
        sleep(1);
    }

    void init();
    void sendData();
    QString getCurrentError() { return this->m_error; }
    void setSettings(StreamSettings settings) { m_settings = settings; init(); }
    SensorStatus getSensorStatus()
    {
        m_error = device->getCurrentError();
//        qDebug()<<Q_FUNC_INFO<<m_error<<m_error.isEmpty()<<m_status;
        if(!m_error.isEmpty())
            m_status = NOT_AVAIL;
        return m_status;
    }
    QMutex mutex;

signals:
    void signal_dataReceived(QByteArray data);
    void signal_dataEncoded(QByteArray data);

protected:
    StreamSettings m_settings;
    QByteArray m_data;
    QString m_error;
    SensorStatus m_status;
    quint8 m_data_error_tick;

    virtual void decode() = 0;
    virtual QByteArray encode() = 0;
    virtual void update() = 0;
    void loop();

private:
    StreamDevice *device;
    bool exitReq;
};

#endif // STREAM_H
