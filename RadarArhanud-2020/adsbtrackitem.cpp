#include "adsbtrackitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <qmath.h>

AdsbTrackItem::AdsbTrackItem(AdsbArhnd::ADSBTargetData* Atarget) :
    m_adsb_target(Atarget)
{
    itemType = RadarSceneItems::ADSB;

    qDebug()<<Q_FUNC_INFO<<Atarget;
    setZValue(1);
}

QRectF AdsbTrackItem::boundingRect() const
{
    qreal pixel_line_velocity = PIXEL_PER_KNOT*10.0;

    return QRectF( -20, -20-pixel_line_velocity, 40, 40+pixel_line_velocity);
    //    return QRectF( -20, -20-pixel_line_velocity, 52, 40+pixel_line_velocity);
}

QPainterPath AdsbTrackItem::shape() const
{
    qreal pixel_line_velocity = PIXEL_PER_KNOT*10.0;
    QPainterPath path;

    path.addRect(-20, -20-pixel_line_velocity, 40, 40+pixel_line_velocity);
    path.addRect(22, -20, 10, 5);
    return path;
}

void AdsbTrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if(state_radar != RADAR_TRANSMIT)
        return;

    QPen pen;

    pen.setWidth(3);
    pen.setColor(Qt::yellow);
    if(map_settings.show && map_settings.mode == 0)
        pen.setColor(Qt::yellow);
    else if(map_settings.show && map_settings.mode == 1)
        pen.setColor(Qt::black);


    //    qDebug()<<Q_FUNC_INFO<<"adsb_item ptr"<<m_adsb_target;
    //    qDebug()<<Q_FUNC_INFO<<"adsb_item rng"<<m_adsb_target->rng;
    //    qDebug()<<Q_FUNC_INFO<<"adsb_item brn"<<m_adsb_target->brn;
    //    qDebug()<<Q_FUNC_INFO<<"adsb_item speed"<<m_adsb_target->speed;
    //    qDebug()<<Q_FUNC_INFO<<"adsb_item alt"<<m_adsb_target->alt;

    painter->setPen(pen);
    painter->drawText(22,-20,"TN: "+QString::number(m_adsb_target->number));
    //    painter->drawText(22,-20,"ID: "+QString::number(m_adsb_target->icao,16).toUpper());
    bool pass = m_adsb_target->lat_valid && m_adsb_target->lon_valid
            && m_adsb_target->alt_valid && adsb_settings.show_track;

    //    if(m_adsb_target->icao == 11260195)
    //    {
    //        pass = true;
    //        m_adsb_target->alt = 2200.;
    //    }

    if(pass)
    {
        painter->drawText(22,-5,"Rng: "+QString::number(m_adsb_target->rng,'f',1)+" Km");
        painter->drawText(22,10,"Brn: "+QString::number(m_adsb_target->brn,'f',1)+176);
        if(m_adsb_target->speed_valid)
            painter->drawText(22,25,"Spd: "+QString::number(m_adsb_target->speed,'f',1)+" kts");
        else
            painter->drawText(22,25,"Spd: - kts");
        painter->drawText(22,40,"Alt: "+QString::number(m_adsb_target->alt*0.305/1000.,'f',1)+" Km");
    }
    if(item_selected)
        painter->drawRect(boundingRect());
    if(m_adsb_target->course_valid) painter->rotate(m_adsb_target->course);
    //    qreal pixel_line_velocity = PIXEL_PER_KNOT*m_adsb_target->speed;
    //    painter->drawLine(0,20,0,pixel_line_velocity);
    //    painter->rotate(-45.);
    //    painter->drawPixmap(-20,-20,40,40,QPixmap(":/images/airplane.png"));

    QString cat, ident;

    //    m_adsb_target->identity = 1; //test
    //    m_adsb_target->cat = AdsbArhnd::ADSBTargetData::AircraftCategory::RWD; //test

    switch (m_adsb_target->identity) {
    case 1:
        ident = "FRIEND";
        pen.setColor(QColor(8, 250, 249));
        break;
    case 2:
        ident = "HOSTILE";
        pen.setColor(QColor(250, 0, 0));
        break;
    default:
        ident = "UNKNOWN";
        pen.setColor(Qt::yellow);
        break;
    }

    switch (m_adsb_target->cat)
    {
    case AdsbArhnd::ADSBTargetData::AircraftCategory::FWD:
        cat = "2";
        break;
    case AdsbArhnd::ADSBTargetData::AircraftCategory::RWD:
        cat = "3";
        break;
    case AdsbArhnd::ADSBTargetData::AircraftCategory::UAV:
        cat = "4";
        break;
    default:
        cat = "1";
        break;
    }

    //    qDebug()<<Q_FUNC_INFO<<"adsb_item m_adsb_target->identity"<<m_adsb_target->identity;
    //    qDebug()<<Q_FUNC_INFO<<"adsb_item m_adsb_target->cat"<<m_adsb_target->cat;

    painter->drawPixmap(-20,-20,40,40,QPixmap(QString(":/images/DATA_SIMBOL_SASARAN/%1/%2.png").arg(ident).arg(cat)));
    //    pen.setColor(Qt::yellow);

    painter->setPen(pen);
    painter->drawLine(0,20,0,30);
}
