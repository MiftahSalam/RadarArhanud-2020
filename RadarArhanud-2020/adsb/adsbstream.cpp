#include "adsbstream.h"

#include <QDebug>
#include <QDataStream>
#include <math.h>
#include <limits>

ADSBStream::ADSBStream(QObject *parent, StreamSettings inSettings) :
    QObject(parent), inStream(new ADSBStreamIn(0,inSettings))
{
    m_InError = inStream->getCurrentError();

    connect(inStream,SIGNAL(signal_dataReceived(QByteArray)),this,SIGNAL(signal_sendStreamData(QByteArray)));
    connect(inStream,SIGNAL(signal_newTarget(int)),this,SLOT(trigger_updateTargetData(int)));
}

void ADSBStream::trigger_updateTargetData(int icao)
{
    /**/
    ADSBTargetData* curTarget = getADSB().getTarget(icao);
    QByteArray data_out;
    QDataStream stream_out(&data_out,QIODevice::WriteOnly);

    /*
    qDebug()<<Q_FUNC_INFO<<"curTarget icao"<<QString::number((qint32)curTarget->icao,16);
    qDebug()<<Q_FUNC_INFO<<"curTarget->CallSign"<<curTarget->trimmed_call_sign;
    qDebug()<<Q_FUNC_INFO<<"curTarget->Lat"<<curTarget->lat;
    qDebug()<<Q_FUNC_INFO<<"curTarget->Lon"<<curTarget->lon;
    qDebug()<<Q_FUNC_INFO<<"curTarget->alt"<<curTarget->alt;
    qDebug()<<Q_FUNC_INFO<<"curTarget->COG"<<curTarget->course;
    qDebug()<<Q_FUNC_INFO<<"curTarget->SOG"<<curTarget->speed;
    qDebug()<<Q_FUNC_INFO<<"curTarget->vertiacal rate"<<curTarget->vertical_rate;
    qDebug()<<Q_FUNC_INFO<<"curTarget->ground"<<curTarget->ground;
    qDebug()<<Q_FUNC_INFO<<"curTarget->contry"<<curTarget->trimmed_country;
    qDebug()<<Q_FUNC_INFO<<"curTarget->time_stamp"<<curTarget->time_stamp;
    qDebug()<<Q_FUNC_INFO<<"curTarget->lat_valid"<<curTarget->lat_valid;
    qDebug()<<Q_FUNC_INFO<<"curTarget->lon_valid"<<curTarget->lon_valid;
    qDebug()<<Q_FUNC_INFO<<"curTarget->alt_valid"<<curTarget->alt_valid;
    qDebug()<<Q_FUNC_INFO<<"curTarget->speed_valid"<<curTarget->speed_valid;
    qDebug()<<Q_FUNC_INFO<<"curTarget->course_valid"<<curTarget->course_valid;
    qDebug()<<Q_FUNC_INFO<<"curTarget->vertical_rate_valid"<<curTarget->vertical_rate_valid;
    */


    float lat = (float)curTarget->lat;
    float lon = (float)curTarget->lon;
    float alt = (float)curTarget->alt*0.3048; //meter
    float cog = (float)curTarget->course; //deg
    float sog = (float)curTarget->speed; // kts
    const float MAX_FLOAT = std::numeric_limits<float>::max();

    stream_out.setByteOrder(QDataStream::LittleEndian);

    if(curTarget->icao) //4 byte
        stream_out<<(quint32)curTarget->icao;
    else
        stream_out<<(quint32)UINT_MAX;

    stream_out.writeRawData((const char*)&curTarget->call_sign,10);  //10 byte

    if(curTarget->lat_valid && curTarget->lon_valid)
    {
        stream_out.writeRawData((const char*)&lat,4);  //4 byte
        stream_out.writeRawData((const char*)&lon,4);  //4 byte
    }
    else
    {
        stream_out.writeRawData((const char*)&MAX_FLOAT,4);  //4 byte
        stream_out.writeRawData((const char*)&MAX_FLOAT,4);  //4 byte
    }

    if(curTarget->alt_valid)
        stream_out.writeRawData((const char*)&alt,4);  //4 byte
    else
        stream_out.writeRawData((const char*)&MAX_FLOAT,4);  //4 byte

    if(curTarget->speed_valid)
        stream_out.writeRawData((const char*)&sog,4);  //4 byte
    else
        stream_out.writeRawData((const char*)&MAX_FLOAT,4);  //4 byte

    if(curTarget->course_valid)
        stream_out.writeRawData((const char*)&cog,4);  //4 byte
    else
        stream_out.writeRawData((const char*)&MAX_FLOAT,4);  //4 byte

    stream_out.writeRawData((const char*)&curTarget->country,10);  //10 byte

    emit signal_updateTargetData(data_out);
}

void ADSBStream::setInSettings(StreamSettings inSettings)
{
    inStream->setSettings(inSettings);
    m_InError = inStream->getCurrentError();
}

ADSBDecoder ADSBStream::getADSB()
{
    return inStream->getADSB();
}

ADSBStream::~ADSBStream()
{
    delete inStream;
}

ADSBStreamIn::ADSBStreamIn(Stream *parent, StreamSettings settings) :
    Stream(parent,settings)
{
    settings.mode = In;
    m_settings = settings;
}
ADSBDecoder ADSBStreamIn::getADSB()
{
    mutex.lock();
    ADSBDecoder dec_adsb = adsbDecoder;
    mutex.unlock();
    return dec_adsb;
}

void ADSBStreamIn::decode()
{
    QList<int> targets = adsbDecoder.decode(adsbParser.parseData(m_data));

//    qDebug()<<Q_FUNC_INFO<<m_data;

    if(!targets.isEmpty())
    {
        m_data_error_tick = 0;
        m_status = AVAIL;
        foreach (const int icao, targets)
        {
            emit signal_newTarget(icao);
            msleep(50);
        }
    }
    else
        m_data_error_tick++;

    if(m_data_error_tick > 200)
    {
        m_data_error_tick = 200;
        m_status = DATA_UNKNOWN;
    }

}

void ADSBStreamIn::update()
{
    adsbDecoder.updateADSB();
}

void ADSBStreamIn::run()
{
    /*
    */
    loop();
}
