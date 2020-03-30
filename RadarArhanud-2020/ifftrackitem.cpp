#include "ifftrackitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <qmath.h>

IFFTrackItem::IFFTrackItem()
{
    itemType = RadarSceneItems::IFF;

    qDebug()<<Q_FUNC_INFO;
    setZValue(1);
}

QRectF IFFTrackItem::boundingRect() const
{
    qreal pixel_line_velocity = PIXEL_PER_KNOT*10.0;

    return QRectF( -20, -20-pixel_line_velocity, 52, 40+pixel_line_velocity);
}

QPainterPath IFFTrackItem::shape() const
{
    qreal pixel_line_velocity = PIXEL_PER_KNOT*10.0;
    QPainterPath path;

    path.addRect(-20, -20-pixel_line_velocity, 40, 40+pixel_line_velocity);
    path.addRect(22, -20, 10, 5);
    return path;
}

void IFFTrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QPen pen;
    pen.setColor(Qt::yellow);

    painter->setPen(pen);
    painter->drawText(22,-20,"Quadron-31");
    painter->rotate(170.0);
    painter->drawPixmap(-20,-20,40,40,QPixmap(":/images/airplane_militer.png"));

    qreal pixel_line_velocity = PIXEL_PER_KNOT*20.0;

    prepareGeometryChange();

    painter->drawLine(0,-20,0,-pixel_line_velocity);

    /*
    QPen pen;
    pen.setColor(Qt::yellow);

    if(m_arpa_target->getStatus() < 5 && m_arpa_target->getStatus() > 0)
    {
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(-10,-10,20,20);
    }
    else if(m_arpa_target->getStatus() > 4)
    {
        painter->setPen(pen);

        painter->drawLine(QPoint(-10,-5),QPoint(-10,-10));
        painter->drawLine(QPoint(-10,-10),QPoint(-5,-10));

        painter->drawLine(QPoint(5,-10),QPoint(10,-10));
        painter->drawLine(QPoint(10,-10),QPoint(10,-5));

        painter->drawLine(QPoint(10,5),QPoint(10,10));
        painter->drawLine(QPoint(10,10),QPoint(5,10));

        painter->drawLine(QPoint(-5,10),QPoint(-10,10));
        painter->drawLine(QPoint(-10,10),QPoint(-10,5));

        qreal pixel_line_velocity = PIXEL_PER_KNOT*m_arpa_target->m_speed_kn;

        painter->drawLine(0,0,
                          pixel_line_velocity*qCos(deg2rad(m_arpa_target->m_course-90.0)),
                          pixel_line_velocity*qSin(deg2rad(m_arpa_target->m_course-90.0))
                          );
        painter->drawText(12,-10,QString::number(m_arpa_target->m_target_id));
    }
    */
}

