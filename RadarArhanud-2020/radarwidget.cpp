#include <QOpenGLFunctions_3_0>
#include <stdlib.h>
#include <math.h>

#include "radarwidget.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

RadarWidget::RadarWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_ri = new RI(this);
    spokeDrawer = RD::make_Draw(m_ri,0);

    connect(m_ri,SIGNAL(signal_plotRadarSpoke(int,int,u_int8_t*,size_t)),
            this,SLOT(trigger_DrawSpoke(int,int,u_int8_t*,size_t)));
    /*

    arpa = new RadarArpa(this,ri);

    timer  = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeOut()));
    timer->start(100);
    */

    setAutoFillBackground(false);
//    setMinimumSize(200, 200);

//    installEventFilter(this);

    /*
    old_motion_mode = radar_settings.headingUp;
    curRange = 0;
    cur_arpa_id_count = 0;
    cur_arpa_number = 0;
    arpa_measure_time = QDateTime::currentMSecsSinceEpoch();
    */

    timerId = startTimer(1000);
}
void RadarWidget::trigger_DrawSpoke(int transparency, int angle, quint8 *data, size_t len)
{
//    qDebug()<<Q_FUNC_INFO;
    spokeDrawer->ProcessRadarSpoke(transparency,angle,data,len);
    update();
}

/*
void RadarWidget::trigger_ReqDelTrack(int id)
{
    if(id>-10)
    {
        for(int i=0;i<arpa->m_number_of_targets;i++)
            if(arpa->m_target[i]->m_target_id == id)
                arpa->m_target[i]->SetStatusLost();
    }
    else
        arpa->DeleteAllTargets();
}
*/
/*
void RadarWidget::timeOut()
{
    if(arpa->m_number_of_targets > 0)
    {
        int num_limit = 5;
        while ((cur_arpa_id_count < arpa->m_number_of_targets) && num_limit > 0)
        {
            if(arpa->m_target[cur_arpa_id_count]->m_target_id > 0)
            {
                Position own_pos;
                own_pos.lat = currentOwnShipLat;
                own_pos.lon = currentOwnShipLon;
                Polar pol = Pos2Polar(arpa->m_target[cur_arpa_id_count]->m_position,own_pos,curRange);
                double brn = SCALE_RAW_TO_DEGREES2048(pol.angle);
                double range = (double)curRange*pol.r/RETURNS_PER_LINE/1000;

                emit signal_target_param(arpa->m_target[cur_arpa_id_count]->m_target_id,
                                         arpa->m_target[cur_arpa_id_count]->m_speed_kn,
                                         arpa->m_target[cur_arpa_id_count]->m_course,
                                         range,
                                         brn
                                         );
            }
            cur_arpa_id_count++;
            num_limit--;
        }
        if(cur_arpa_id_count >= arpa->m_number_of_targets)
            cur_arpa_id_count = 0;
    }

    if(old_motion_mode^radar_settings.headingUp)
    {
        arpa->DeleteAllTargets();
        old_motion_mode = radar_settings.headingUp;
    }

    update();
}
*/

void RadarWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

//    qDebug()<<Q_FUNC_INFO<<geometry();

    update();
}

void RadarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    makeCurrent();

    QOpenGLFunctions_3_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_0>();
    f->glMatrixMode(GL_MODELVIEW);
    f->glPushMatrix();

    f->glClearColor(0.,0.,0.,0.);
    f->glEnable(GL_MULTISAMPLE);

    f->glShadeModel(GL_SMOOTH);
    f->glEnable(GL_DEPTH_TEST);

//    f->glEnable(GL_BLEND); //tambahan tes blend

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f->glLoadIdentity();
    f->glTranslatef(0.0, 0.0, -10.0);
    f->glScaled(.5, .5, 1.);

//    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); //tambahan tes blend

    spokeDrawer->DrawRadarImage();

    f->glShadeModel(GL_FLAT);
    f->glDisable(GL_DEPTH_TEST);

//    glDisable(GL_BLEND); //tambahan tes blend

    f->glMatrixMode(GL_MODELVIEW);
    f->glPopMatrix();

    emit signal_updateRadarEcho();
//    qDebug()<<Q_FUNC_INFO;
}

void RadarWidget::resizeGL(int width, int height)
{
//    qDebug()<<Q_FUNC_INFO;

    setupViewport(width, height);
}

RadarWidget::~RadarWidget()
{
}

void RadarWidget::initializeGL()
{
    qDebug()<<Q_FUNC_INFO;

//    glEnable(GL_MULTISAMPLE);
}

void RadarWidget::setupViewport(int width, int height)
{
//    qDebug()<<Q_FUNC_INFO<<width<<height;

    QOpenGLFunctions_3_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_0>();
    int side = qMin(width, height);

    f->glViewport((width - side) / 2, (height - side) / 2, side, side);

    f->glMatrixMode(GL_PROJECTION);
    f->glLoadIdentity();
#ifdef QT_OPENGL_ES
    f->glOrthof(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#else
    f->glOrtho(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#endif
    f->glMatrixMode(GL_MODELVIEW);
}

