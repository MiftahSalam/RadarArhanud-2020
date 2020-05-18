#include "adsbtrackitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <qmath.h>

AdsbTrackItem::AdsbTrackItem(ADSBTargetData* Atarget) :
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

    painter->setPen(pen);
    painter->drawText(22,-20,QString::number(m_adsb_target->icao,16));
    painter->rotate(m_adsb_target->course);
    painter->drawPixmap(-20,-20,40,40,QPixmap(":/images/airplane1.png"));

    qreal pixel_line_velocity = PIXEL_PER_KNOT*m_adsb_target->speed;

    painter->drawLine(0,-20,0,-pixel_line_velocity);
}

