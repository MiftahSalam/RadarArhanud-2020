#ifndef RADARGRAPHICVIEW_H
#define RADARGRAPHICVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>
#include <QImage>
#include <QTimer>

#include <mapcontrol.h>
#include <maplayer.h>
#include <geometrylayer.h>
#include <googlemapadapter.h>
#include <osmmapadapter.h>
#include <emptymapadapter.h>
#include <fixedimageoverlay.h>

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

    qreal calculateRangeRing() const;
    void setMapZoomLevel(int index);
    void tesCreateItem(); //temporary

signals:
    void signal_mapChange(QImage img);
    void signal_cursorPosition(qreal lat, qreal lon, qreal rng, qreal brn);
    void signal_reqCreateArpa(QPointF position);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onTimeOut();
    void trigger_mapChange(quint8 id, quint8 val);

private:
    MapControl *mc;
    MapLayer *l;
    MapAdapter* mapadapter;
    QPointF mapCenter;
    QImage mapImage;
    QTimer *timer;
    int curLoadingMapSize;
    double curLat,curLon;

    void updateSceneItems();
};

#endif // RADARGRAPHICVIEW_H
