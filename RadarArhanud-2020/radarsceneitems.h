#ifndef RADARSCENEITEMS_H
#define RADARSCENEITEMS_H

#include <QGraphicsItem>

#include <radarengine.h>

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
    void setShow(bool shown);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    RadarItemsType itemType;
//    bool show;
};

#endif // RADARSCENEITEMS_H
