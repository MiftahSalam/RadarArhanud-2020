#include "radarsceneitems.h"
#include <QDebug>

RadarSceneItems::RadarSceneItems()
{
    setFlag(ItemIsSelectable);
}

QVariant RadarSceneItems::itemChange(GraphicsItemChange change, const QVariant &value)
{
//    qDebug()<<Q_FUNC_INFO<<change<<value;
    switch (change) {
    case ItemSelectedChange:
        qDebug()<<Q_FUNC_INFO<<change;
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}


