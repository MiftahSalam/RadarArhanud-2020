#include "arpatrackitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

ArpaTrackItem::ArpaTrackItem(ARPATarget *ATarget):
    m_arpa_target(ATarget)
{
    qDebug()<<Q_FUNC_INFO<<m_arpa_target->m_target_id;
    setFlag(ItemIsSelectable);
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

#include <QRadialGradient>
void ArpaTrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
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

        painter->drawText(12,-10,QString::number(m_arpa_target->m_target_id));
    }
}

QVariant ArpaTrackItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        qDebug()<<Q_FUNC_INFO<<"ItemPositionHasChanged";
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}



