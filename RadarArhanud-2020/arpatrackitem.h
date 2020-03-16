#ifndef ARPATRACKITEM_H
#define ARPATRACKITEM_H

#include <QGraphicsItem>

class ArpaTrackItem : public QGraphicsItem
{
public:
    explicit ArpaTrackItem();

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

signals:

public slots:
};

#endif // ARPATRACKITEM_H
