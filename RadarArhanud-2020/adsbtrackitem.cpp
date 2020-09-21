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

    return QRectF( -20, -20-pixel_line_velocity, 52, 40+pixel_line_velocity);
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
    QPen pen;

    pen.setColor(Qt::yellow);
    if(map_settings.show && map_settings.mode == 0)
        pen.setColor(Qt::yellow);
    else if(map_settings.show && map_settings.mode == 1)
        pen.setColor(Qt::black);


    painter->setPen(pen);
    painter->drawText(22,-20,"ID: "+QString::number(m_adsb_target->icao,16).toUpper());
    if(m_adsb_target->lat_valid && m_adsb_target->lon_valid && adsb_settings.show_attr)
    {
        painter->drawText(22,-5,"Rng: "+QString::number(m_adsb_target->rng,'f',1)+" Km");
        painter->drawText(22,10,"Brn: "+QString::number(m_adsb_target->brn,'f',1)+176);
        painter->drawText(22,25,"Spd: "+QString::number(m_adsb_target->speed,'f',1)+" kts");
        painter->drawText(22,40,"Alt: "+QString::number(m_adsb_target->alt/1000.,'f',1)+" Km");
    }
    painter->rotate(m_adsb_target->course);
//    qreal pixel_line_velocity = PIXEL_PER_KNOT*m_adsb_target->speed;
//    painter->drawLine(0,20,0,pixel_line_velocity);
    painter->rotate(-45.);
    painter->drawPixmap(-20,-20,40,40,QPixmap(":/images/airplane.png"));
//    painter->drawPixmap(-20,-20,40,40,QPixmap(":/images/airplane1.png"));


}

