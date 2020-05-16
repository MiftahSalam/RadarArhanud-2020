#ifndef ADSBTRACKITEM_H
#define ADSBTRACKITEM_H

#include <QGraphicsItem>

#include "radarsceneitems.h"

class AdsbTrackItem : public RadarSceneItems
{
public:
    explicit AdsbTrackItem();

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

};

#endif // ADSBTRACKITEM_H
