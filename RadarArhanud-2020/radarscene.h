#ifndef RADARSCENE_H
#define RADARSCENE_H

#include <QGraphicsScene>
#include <QTimer>
#include <QGLBuffer>
#include <QGLShader>

#include "radarengine.h"
#include "adsb/adsbstream.h"

using namespace RadarEngineARND;

class RadarScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit RadarScene(QObject *parent = 0, RadarEngine *ri_ptr=0);
    ~RadarScene();

    void DrawSpoke(int, u_int8_t*, size_t);
    void reqNewArpa(bool create, ARPATarget *arpa_ptr);
    void reqNewADSB(AdsbArhnd::ADSBTargetData *target);
    void setRadarScale(float scale);
    void drawBackground(QPainter *painter, const QRectF &) override;
//    void drawForeground(QPainter *painter, const QRectF &) override;

signals:

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void trigger_mapChange(QImage img);
    void trigger_cursorPosition(qreal lat, qreal lon, qreal rng, qreal brn);

private:
    RadarEngine *m_ri;
    QTimer *m_timer;

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

    float curScale;

    QImage mapImage;

    void saveGLState();
    void restoreGLState();
    void initGL();
};

#endif // RADARSCENE_H
