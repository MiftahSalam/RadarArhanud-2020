#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QTimer>

/*
#include "simrad_global.h"
#include "RadarDraw.h"
#include "radarmarpa.h"
*/

class RadarWidget : public QGLWidget
{
    Q_OBJECT

public:
    RadarWidget(QWidget *parent = 0/*,RadarInfo *ri = 0*/);
    ~RadarWidget();

//    void setRectRegoin(QRect rect);
//    void setRange(int range);
//    void computetRingWidth();

//    double getRingWidth();

signals:
    void signal_target_param(int id,double spd, double crs, double lat, double lon);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    /*
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    */

private slots:
//    void timeOut();
//    void trigger_DrawSpoke(int transparency, int angle, u_int8_t* data, size_t len);
//    void trigger_ReqDelTrack(int id);

private:
    void setupViewport(int width, int height);
//    void createMARPA(QPoint pos);

    /*
    RadarDraw *spokeDrawer;
    RadarInfo *m_ri;
    RadarArpa *arpa;
    QTimer *timer;
    QRect region;

    double ringWidth;
    int curRange;
    int cur_arpa_id_count;
    int cur_arpa_number;
    quint64 arpa_measure_time;
    bool old_motion_mode;
    */
};

#endif
