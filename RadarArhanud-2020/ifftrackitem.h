#ifndef IFFTRACKITEM_H
#define IFFTRACKITEM_H

#include <QGraphicsItem>

#include "radarsceneitems.h"

class IFFTrackItem : public RadarSceneItems
{
public:
    explicit IFFTrackItem();

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

};

#endif // IFFTRACKITEM_H
