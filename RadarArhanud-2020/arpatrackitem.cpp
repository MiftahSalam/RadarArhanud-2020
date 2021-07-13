#include "arpatrackitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <qmath.h>

ArpaTrackItem::ArpaTrackItem(TracksCluster *ATarget):
    m_cluster_track(ATarget)
//    ArpaTrackItem::ArpaTrackItem(RadarEngineARND::ARPATarget *ATarget):
//    m_arpa_target(ATarget)
{
    itemType = RadarItemsType::ARPA;
    m_arpa_target = m_cluster_track->getARPATargetClustered();
    qDebug()<<Q_FUNC_INFO<<m_arpa_target;
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

    m_arpa_target = m_cluster_track->getARPATargetClustered();
    if(m_arpa_target == nullptr)
        return;

    if(m_cluster_track->getClusterTrackStatus() < 5 && m_cluster_track->getClusterTrackStatus() > 0)
//        if(m_arpa_target->getStatus() < 5 && m_arpa_target->getStatus() > 0)
    {
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(-20,-20,30,30);
    }
    else if(m_cluster_track->getClusterTrackStatus() > 4)
//        else if(m_arpa_target->getStatus() > 4)
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
        painter->rotate(m_arpa_target->m_course);
        painter->drawPixmap(-20,-20,40,40,QPixmap(":/images/DATA_SIMBOL_SASARAN/UNKNOWN/2.png"));
        painter->drawLine(0,20,0,30);
        painter->rotate(-m_arpa_target->m_course);

        /*
        qreal pixel_line_velocity = PIXEL_PER_KNOT*m_arpa_target->m_speed_kn;

        painter->drawLine(0,0,
                          pixel_line_velocity*qCos(deg2rad(m_arpa_target->m_course-90.0)),
                          pixel_line_velocity*qSin(deg2rad(m_arpa_target->m_course-90.0))
                          );
        */
        painter->drawText(
                    22,
                    -20,
                    "TN: "+QString::number(m_arpa_target->m_target_number)+
                    ", anId: "+QString::number(m_cluster_track->getClusterAntheneId()) //debug
                    );
//        painter->drawText(22,-20,"ID: "+QString::number(m_arpa_target->m_target_id));
        if(arpa_settings[0].show_attr) //semetara
        {
            painter->drawText(22,-5,"Rng: "+QString::number(m_arpa_target->m_position.rng,'f',1)+" Km");
            painter->drawText(22,10,"Brn: "+QString::number(m_arpa_target->m_position.brn,'f',1)+176);
            painter->drawText(22,25,"Spd: "+QString::number(m_arpa_target->m_position.speed_kn,'f',1)+" kts");

            float hight = m_arpa_target->m_position.alt/1000.;
            painter->drawText(22,40,"Alt: "+QString::number(hight,'f',1)+" Km");

        }
        if(item_selected)
            painter->drawRect(boundingRect());
    }
}
