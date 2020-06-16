#ifndef RADARSCENEITEMS_H
#define RADARSCENEITEMS_H

#include <QGraphicsItem>

#include "echo/radar_global.h"

#define PIXEL_PER_KNOT 1.0

class RadarSceneItems : public QGraphicsItem
{
public:
    RadarSceneItems();

    enum RadarItemsType
    {
        ARPA,
        IFF,
        ADSB
    };

    RadarItemsType getRadarItemType() { return itemType; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    RadarItemsType itemType;
};

#endif // RADARSCENEITEMS_H
