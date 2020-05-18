#ifndef ADSBTRACKITEM_H
#define ADSBTRACKITEM_H

#include <QGraphicsItem>

#include "radarsceneitems.h"
#include "adsb/adsbstream.h"

class AdsbTrackItem : public RadarSceneItems
{
public:
    explicit AdsbTrackItem(ADSBTargetData *ATarget=0);

    int getTargetIcao() { return m_adsb_target->icao; }
    ADSBTargetData *m_adsb_target;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

};

#endif // ADSBTRACKITEM_H
