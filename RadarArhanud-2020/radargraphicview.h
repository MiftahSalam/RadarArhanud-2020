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

#include "arpatrackitem.h"
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

protected:
    void resizeEvent(QResizeEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    /*
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    */

private slots:
    void onTimeOut();
    /*
    void trigger_ReqDelTrack(int id);
    */

private:
    MapControl *mc;
    MapLayer *l;
    MapAdapter* mapadapter;

    ArpaTrackItem *tr1;
    RadarWidget *echo;

    QTimer timer;

//    bool loadMapFinish;

//    void loadMap();
};

#endif // RADARGRAPHICVIEW_H
