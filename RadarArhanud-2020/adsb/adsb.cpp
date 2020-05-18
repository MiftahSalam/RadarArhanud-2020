#include "adsb.h"

#include <QDebug>
#include <QDateTime>
#include <math.h>

ADSBDecoder::ADSBDecoder()
{
    cur_targets_icao.clear();
    targetListMap.clear();
}

void ADSBDecoder::updateADSB()
{
//    qDebug()<<Q_FUNC_INFO<<"targetListMap->size"<<targetListMap.size();

    QHashIterator<int,ADSBTargetData*> i(targetListMap);

    while(i.hasNext())
    {
        i.next();

        ADSBTargetData *buf_data = i.value();

        if(IsExpired(buf_data->icao))
        {
            deleteTarget(buf_data->icao);
            continue;
        }
    }
}

bool ADSBDecoder::IsExpired(int icao)
{
    QDateTime time = QDateTime::currentDateTime();

    return (time.toTime_t() - targetListMap.value(icao)->time_stamp > ADSB_TARGET_EXPIRED);
}

void ADSBDecoder::deleteTarget(int icao)
{
    targetListMap.remove(icao);
//    qDebug()<<Q_FUNC_INFO<<icao;
}

QList<int> ADSBDecoder::decode(QJsonArray targets)
{
    cur_targets_icao.clear();

//    qDebug()<<Q_FUNC_INFO<<targets.size();

    for(int i=0; i<targets.size(); i++)
    {
        QJsonObject target = targets.at(i).toObject();

        bool ok;
        int icao = target.value("hex").toString("-1").toInt(&ok,16);

        if(ok && (icao > 0))
        {
            ADSBTargetData *cur_target;

            if(targetListMap.contains(icao))
                cur_target = targetListMap.take(icao);
            else
                cur_target = new ADSBTargetData();

            QString call_sign = target.value("fli").toString("");
            float lat = target.value("lat").toDouble(1000);
            float lon = target.value("lon").toDouble(1000);
            float alt = target.value("alt").toDouble(1000000);
            float speed = target.value("spd").toDouble(10000);
            float course = target.value("trk").toDouble(1000);
            float vertical_rate = target.value("vrt").toDouble(10000);
            quint8 ground = target.value("gda").toString("G").contains("G") ? 0 : 1;
            QString country = target.value("cou").toString("");

            cur_target->icao = icao;
            cur_target->trimmed_call_sign = call_sign;
            strncpy(cur_target->call_sign,call_sign.toUtf8().constData(),call_sign.size());
            cur_target->lat = lat;
            cur_target->lon = lon;
            cur_target->alt = alt;
            cur_target->speed = speed;
            cur_target->course = course;
            cur_target->vertical_rate = vertical_rate;
            cur_target->ground = ground;
            cur_target->trimmed_country = country;
            strncpy(cur_target->country,country.toUtf8().constData(),country.size());

            if(fabs(lat) <= 90)
                cur_target->lat_valid = true;
            else
                cur_target->lat_valid = false;

            if(fabs(lon) <= 180)
                cur_target->lon_valid = true;
            else
                cur_target->lon_valid = false;

            if(fabs(alt) < 1000000)
                cur_target->alt_valid = true;
            else
                cur_target->alt_valid = false;

            if(fabs(speed) < 10000)
                cur_target->speed_valid = true;
            else
                cur_target->speed_valid = false;

            if(fabs(course) <= 360)
                cur_target->course_valid = true;
            else
                cur_target->course_valid = false;

            if(fabs(vertical_rate) <= 10000)
                cur_target->vertical_rate_valid = true;
            else
                cur_target->vertical_rate_valid = false;

            cur_target->time_stamp = QDateTime::currentDateTime().toTime_t();
            /*
            qDebug()<<Q_FUNC_INFO<<"icao"<<icao;
            qDebug()<<Q_FUNC_INFO<<"cur_target->call_sign"<<cur_target->call_sign<<call_sign.size();
            qDebug()<<Q_FUNC_INFO<<"cur_target->lat"<<cur_target->lat;
            qDebug()<<Q_FUNC_INFO<<"cur_target->lon"<<cur_target->lon;
            qDebug()<<Q_FUNC_INFO<<"cur_target->alt"<<cur_target->alt;
            qDebug()<<Q_FUNC_INFO<<"cur_target->speed"<<cur_target->speed;
            qDebug()<<Q_FUNC_INFO<<"cur_target->course"<<cur_target->course;
            qDebug()<<Q_FUNC_INFO<<"cur_target->vertical_rate"<<cur_target->vertical_rate;
            qDebug()<<Q_FUNC_INFO<<"cur_target->ground"<<cur_target->ground;
            qDebug()<<Q_FUNC_INFO<<"cur_target->country"<<cur_target->country<<country.size();
            qDebug()<<Q_FUNC_INFO<<"cur_target->time_stamp"<<cur_target->time_stamp;
            */

            targetListMap.insert(icao,cur_target);
            cur_targets_icao.append(icao);
        }
    }
    return cur_targets_icao;

//    qDebug()<<Q_FUNC_INFO<<"targetListMap->size"<<targetListMap.size();
}

ADSBParser::ADSBParser()
{
    appendString.clear();
    start_append = false;
    stop_append = false;
}

QJsonArray ADSBParser::parseData(QByteArray json_data)
{
    QByteArray data = preParsedData(json_data);
    QJsonArray arry_json;

    if(!data.isEmpty())
    {
        QJsonParseError error;
        arry_json = QJsonDocument::fromJson(data,&error).array();
//        qDebug()<<Q_FUNC_INFO<<error.errorString()<<arry_json.size();;

    }
//    qDebug()<<Q_FUNC_INFO<<arry_json.size()<<data;

    return arry_json;
}

QByteArray ADSBParser::preParsedData(QByteArray data)
{
    char buffer = '\0';
    QByteArray ret_string;
    ret_string.clear();

    for(int i=0; i<data.size(); i++)
    {
        buffer = data.at(i);

        if(buffer == '[')
            start_append = true;

        if(buffer == ']' && start_append)
            stop_append = true;

        if(start_append)
            appendString.append(buffer);

        if(start_append && stop_append)
        {
            start_append = false;
            stop_append = false;

//            qDebug()<<Q_FUNC_INFO<<"appendString"<<appendString;
            ret_string = appendString;
            appendString.clear();

        }
    }

    return ret_string;
}

ADSBTargetData::ADSBTargetData()
{
    icao = -1;
    strncpy(call_sign,"@@@@@@@@@@",10);
    lat = 1000;
    lon = 1000;
    alt = 1000000;
    speed = 10000;
    course = 1000;
    vertical_rate = 10000;
    ground = 10;
    strncpy(country,"@@@@@@@@@@",10);
    QDateTime time = QDateTime::currentDateTime();
    time_stamp = time.toTime_t();
//    qDebug()<<Q_FUNC_INFO<<"time_stamp"<<time_stamp;
}
