#ifndef ADSBSTREAM_H
#define ADSBSTREAM_H

#include <QObject>

#include "stream/stream.h"
#include "adsb.h"

class ADSBStreamIn : public Stream
{
    Q_OBJECT
public:
    explicit ADSBStreamIn(Stream *parent = 0, StreamSettings settings = StreamSettings());

    ADSBDecoder getADSB();
signals:
    void signal_newTarget(int icao);

private slots:

protected:
    void decode();
    void update();
    QByteArray encode() {}
    void run();

private:
    ADSBParser adsbParser;
    ADSBDecoder adsbDecoder;
};

class ADSBStream : public QObject
{
    Q_OBJECT
public:
    explicit ADSBStream(QObject *parent = 0,
                        StreamSettings inSettings = StreamSettings());

    ADSBDecoder getADSB();
    QString getCurrentInputError() { return this->m_InError; }
    SensorStatus getCurrentSensorStatus() { return this->inStream->getSensorStatus(); }
    void setInSettings(StreamSettings inSettings);

    ~ADSBStream();

signals:
//    void signal_sendTarget(QByteArray data);
    void signal_sendStreamData(QByteArray data);
    void signal_updateTargetData(QByteArray data);

private slots:
    void trigger_updateTargetData(int icao);

private:
    ADSBStreamIn *inStream;
    QString m_InError;
};

#endif // ADSBSTREAM_H
