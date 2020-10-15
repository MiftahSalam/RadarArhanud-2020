#ifndef RADARSCENEITEMS_H
#define RADARSCENEITEMS_H

#include <QGraphicsItem>

#include <radarengine.h>

#define PIXEL_PER_KNOT 0.
//#define PIXEL_PER_KNOT .25

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
    void setItemSelected(bool selected) { item_selected = selected; }
    bool getItemSelected() const { return item_selected; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    RadarItemsType itemType;
    bool item_selected;
};

#endif // RADARSCENEITEMS_H
