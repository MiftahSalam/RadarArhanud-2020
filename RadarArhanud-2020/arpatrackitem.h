#ifndef ARPATRACKITEM_H
#define ARPATRACKITEM_H

#include <QGraphicsItem>

#include <radarengine.h>
#include "radarsceneitems.h"
#include "trackmanager.h"

class ArpaTrackItem : public RadarSceneItems
{
public:
    explicit ArpaTrackItem(TracksCluster *ATarget);
//    explicit ArpaTrackItem(RadarEngineARND::ARPATarget *ATarget);

    int getArpaId() { return m_arpa_target->m_target_id; }
    RadarEngineARND::ARPATarget *m_arpa_target;
    TracksCluster *m_cluster_track;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:

public slots:

private:
};

#endif // ARPATRACKITEM_H
