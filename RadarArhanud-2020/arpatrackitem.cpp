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
    return QRectF( -20, -20, 42, 30);
}

QPainterPath ArpaTrackItem::shape() const
{
    QPainterPath path;
    path.addRect(-20, -20, 30, 30);
    path.addRect(22, -20, 20, 15);
    return path;
}

void ArpaTrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QPen pen;
    pen.setWidth(3);

    pen.setColor(Qt::yellow);
    if(map_settings.show && map_settings.mode == 0)
        pen.setColor(Qt::yellow);
    else if(map_settings.show && map_settings.mode == 1)
        pen.setColor(Qt::black);

    if(m_arpa_target->getStatus() < 5 && m_arpa_target->getStatus() > 0)
    {
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(-20,-20,30,30);
    }
    else if(m_arpa_target->getStatus() > 4)
    {
        painter->setPen(pen);
//        painter->drawRect(-20,-20,30,30);

        /*
        painter->drawLine(QPoint(-20,-15),QPoint(-20,-20));
        painter->drawLine(QPoint(-20,-20),QPoint(-15,-20));

        painter->drawLine(QPoint(15,-20),QPoint(20,-20));
        painter->drawLine(QPoint(20,-20),QPoint(20,-15));

        painter->drawLine(QPoint(20,15),QPoint(20,20));
        painter->drawLine(QPoint(20,20),QPoint(15,20));

        painter->drawLine(QPoint(-15,20),QPoint(-20,20));
        painter->drawLine(QPoint(-20,20),QPoint(-20,15));
        */
        painter->drawPixmap(-20,-20,40,40,QPixmap(":/images/DATA_SIMBOL_SASARAN/UNKNOWN/1.png"));

        /*
        qreal pixel_line_velocity = PIXEL_PER_KNOT*m_arpa_target->m_speed_kn;

        painter->drawLine(0,0,
                          pixel_line_velocity*qCos(deg2rad(m_arpa_target->m_course-90.0)),
                          pixel_line_velocity*qSin(deg2rad(m_arpa_target->m_course-90.0))
                          );
        */
        painter->drawText(22,-20,"TN: "+QString::number(m_arpa_target->m_target_number));
//        painter->drawText(22,-20,"ID: "+QString::number(m_arpa_target->m_target_id));
        if(arpa_settings.show_attr)
        {
            painter->drawText(22,-5,"Rng: "+QString::number(m_arpa_target->m_position.rng,'f',1)+" Km");
            painter->drawText(22,10,"Brn: "+QString::number(m_arpa_target->m_position.brn,'f',1)+176);
            painter->drawText(22,25,"Spd: "+QString::number(m_arpa_target->m_position.speed_kn,'f',1)+" kts");
        }
        if(item_selected)
            painter->drawRect(boundingRect());
    }
}
