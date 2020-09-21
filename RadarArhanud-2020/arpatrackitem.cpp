#include "arpatrackitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <qmath.h>

ArpaTrackItem::ArpaTrackItem(RadarEngineARND::ARPATarget *ATarget):
    m_arpa_target(ATarget)
{
    itemType = RadarItemsType::ARPA;

    qDebug()<<Q_FUNC_INFO<<m_arpa_target->m_target_id;
    setZValue(1);
}

QRectF ArpaTrackItem::boundingRect() const
{
//    return QRectF( -10, -10, 20, 20);
    return QRectF( -10, -10, 32, 20);
}

QPainterPath ArpaTrackItem::shape() const
{
    QPainterPath path;
    path.addRect(-10, -10, 20, 20);
    path.addRect(12, -10, 10, 5);
    return path;
}

void ArpaTrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QPen pen;

    pen.setColor(Qt::yellow);
    if(map_settings.show && map_settings.mode == 0)
        pen.setColor(Qt::yellow);
    else if(map_settings.show && map_settings.mode == 1)
        pen.setColor(Qt::black);

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

        /*
        qreal pixel_line_velocity = PIXEL_PER_KNOT*m_arpa_target->m_speed_kn;

        painter->drawLine(0,0,
                          pixel_line_velocity*qCos(deg2rad(m_arpa_target->m_course-90.0)),
                          pixel_line_velocity*qSin(deg2rad(m_arpa_target->m_course-90.0))
                          );
        */
        painter->drawText(22,-20,"ID: "+QString::number(m_arpa_target->m_target_id));
        if(arpa_settings.show_attr)
        {
            painter->drawText(22,-5,"Rng: "+QString::number(m_arpa_target->m_position.rng,'f',1)+" Km");
            painter->drawText(22,10,"Brn: "+QString::number(m_arpa_target->m_position.brn,'f',1)+176);
            painter->drawText(22,25,"Spd: "+QString::number(m_arpa_target->m_position.speed_kn,'f',1)+" kts");
        }
    }
}
