#ifndef RADARGRAPHICVIEW_H
#define RADARGRAPHICVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QResizeEvent>
#include <QTimer>

#include <mapcontrol.h>
#include <maplayer.h>
#include <geometrylayer.h>
#include <googlemapadapter.h>
#include <osmmapadapter.h>
#include <emptymapadapter.h>
#include <fixedimageoverlay.h>

#include "radarwidget.h"

using namespace qmapcontrol;

namespace Ui {
class RadarGraphicView;
}

class RadarGraphicView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit RadarGraphicView(QWidget *parent = 0);
    ~RadarGraphicView();

    RI *getRadarInfo() { return echo->m_ri; }

signals:
    void signal_rangeChange(qreal range_rings);
    void signal_reqCreateArpa(QPointF position);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    /*
    */

private slots:
    void onTimeOut();

    void trigger_RangeChange(int zoom_lvl);
    void trigger_mapChange(quint8 id, quint8 val);
    /*
    void trigger_ReqDelTrack(int id);
    */

private:
    MapControl *mc;
    MapLayer *l;
    MapAdapter* mapadapter;

    RadarWidget *echo;

    QTimer timer;
    struct Cursor
    {
        QTime cursorMoveTime;
        qreal latitude;
        qreal longitude;
        qreal range;
        qreal bearing;
    }currentCursor;

    bool loadMapFinish;
    QPointF mapCenter;

    qreal calculateRangeRing();
    void updateArpaItem();
};

#endif // RADARGRAPHICVIEW_H
