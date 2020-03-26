#ifndef ARPATRACKITEM_H
#define ARPATRACKITEM_H

#include <QGraphicsItem>

#include "echo/radar.h"

class ArpaTrackItem : public QGraphicsItem
{
public:
    explicit ArpaTrackItem(ARPATarget *ATarget);

    int getArpaId() { return m_arpa_target->m_target_id; }
    ARPATarget *m_arpa_target;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

signals:

public slots:

private:
};

#endif // ARPATRACKITEM_H
