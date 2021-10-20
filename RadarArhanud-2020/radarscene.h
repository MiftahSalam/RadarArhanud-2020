#ifndef RADARSCENE_H
#define RADARSCENE_H

#include <QGraphicsScene>
#include <QTimer>
#include <QGLBuffer>
#include <QGLShader>

#include "radarengine.h"
#include "trackmanager.h"
#include "adsb/adsbstream.h"

class TracksCluster;

using namespace RadarEngineARND;

class RadarScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit RadarScene(QObject *parent = 0, RadarEngine *ri_ptr=0, RadarEngine *ri_ptr1=0);
    ~RadarScene();

    void DrawSpoke(int, u_int8_t*, size_t);
    void DrawSpoke1(int, u_int8_t*, size_t);
    void reqNewArpa(bool create, bool show, TracksCluster *arpa_cluster_ptr);
//    void reqNewArpa(bool create, bool show, ARPATarget *arpa_ptr);
    void reqDelArpa(TracksCluster *arpa_cluster_ptr);
    void reqNewADSB(AdsbArhnd::ADSBTargetData *target,bool show);
    void setRadarScale(float scale);
    void setRings(int pix) { ringPix = pix; }
    void drawBackground(QPainter *painter, const QRectF &) override;
//    void drawForeground(QPainter *painter, const QRectF &) override;

signals:
    void signal_zero_detect();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void trigger_mapChange(QImage img);
    void trigger_cursorPosition(qreal lat, qreal lon, qreal rng, qreal brn);

private:
    RadarEngine *m_ri,*m_ri1;
    QTimer *m_timer;
    QTime rpm_time;

    struct Cursor
    {
        QTime cursorMoveTime;
        qreal latitude;
        qreal longitude;
        qreal range;
        qreal bearing;
    }currentCursor;

    QGLBuffer vbo;
    QGLShader *m_vertexShader;
    QGLShader *m_fragmentShaders;
    QGLShaderProgram *m_environmentProgram;
    GLTextureCube *m_text;
    QVector<GLfloat> vertData;

    float curScale,curAngle, curAngle1;
    int ringPix, first_sweep_counter, rpm;

    QImage mapImage;

    void saveGLState();
    void restoreGLState();
    void initGL();
};

#endif // RADARSCENE_H
