#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "echo/radar_global.h"
#include "echo/radar.h"

#include <QOpenGLWidget>
#include <QTimerEvent>

class RadarWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    RadarWidget(QWidget *parent = 0);
    ~RadarWidget();

    void setupViewport(int width, int height);

    /*
    void setRectRegoin(QRect rect);
    void setRange(int range);
    void computetRingWidth();

    double getRingWidth();
    */

signals:
    void signal_target_param(int id,double spd, double crs, double lat, double lon);
    void signal_updateRadarEcho();

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent *event) override;
    /*
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    */

private slots:
    void trigger_DrawSpoke(int transparency, int angle, u_int8_t* data, size_t len);
    /*
    void timeOut();
    void trigger_ReqDelTrack(int id);
    */

private:

    RD *spokeDrawer;
    RI *m_ri;

    /*
    void createMARPA(QPoint pos);

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
    int timerId;
};

#endif
