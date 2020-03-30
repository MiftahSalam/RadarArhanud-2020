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
    RadarWidget(QWidget *parent = 0, int map_zoom_level = 0);
    ~RadarWidget();

    void setupViewport(int width, int height);

    RI *m_ri;
    /*
    void setRectRegoin(QRect rect);
    void setRange(int range);

    */

signals:
    void signal_target_param(int id,double spd, double crs, double lat, double lon);
    void signal_updateRadarEcho();
    void signal_zoom_change(int zoom_lvl);
    void signal_arpaChange(bool create, int id);

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
    void trigger_RangeChange(int rng);
    void trigger_reqCreateArpa(QPointF position);
    /*
    void timeOut();
    void trigger_ReqDelTrack(int id);
    */

private:

    RD *spokeDrawer;
    RA *arpa;

    int m_range_meters;
    int m_range_pixel;
    int mapZoomLevel;
    double curRadarScaled;
    /*
    void createMARPA(QPoint pos);

    QTimer *timer;

    int curRange;
    int cur_arpa_id_count;
    int cur_arpa_number;
    quint64 arpa_measure_time;
    bool old_motion_mode;
    */
    int timerId;
};

#endif
