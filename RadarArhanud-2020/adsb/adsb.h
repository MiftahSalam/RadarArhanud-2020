/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */

#ifndef ADSB_H
#define ADSB_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>

const quint64 ADSB_TARGET_EXPIRED = 65;

class ADSBTargetData
{
public:
    ADSBTargetData();

    int icao;
    char call_sign[11];
    QString trimmed_call_sign;
    float lat;
    float lon;
    float alt;
    float speed;
    float course;
    float vertical_rate;
    bool lat_valid;
    bool lon_valid;
    bool alt_valid;
    bool speed_valid;
    bool course_valid;
    bool vertical_rate_valid;
    quint8 ground;
    char country[11];
    QString trimmed_country;
    uint time_stamp;
};

class ADSBParser
{
public:
    ADSBParser();

    QJsonArray parseData(QByteArray json_data);

private:
    QByteArray preParsedData(QByteArray data);

    QByteArray appendString;
    bool start_append;
    bool stop_append;

};

class ADSBDecoder
{
public:
    ADSBDecoder();

    QHash<int,ADSBTargetData*> getTargets() { return targetListMap; }
    ADSBTargetData* getTarget(int icao) const { return targetListMap.value(icao); }
    void updateADSB();
    QList<int> decode(QJsonArray targets);

private:
    QList<int> cur_targets_icao;
    QHash<int,ADSBTargetData*> targetListMap;

    bool IsExpired(int icao);
    void deleteTarget(int icao);
};

#endif
