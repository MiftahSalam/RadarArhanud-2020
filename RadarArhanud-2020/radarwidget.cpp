#include <QtGui>
#include <QtOpenGL>
#include <stdlib.h>
#include <math.h>

#include "radarwidget.h"
//#include "drawutil.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

RadarWidget::RadarWidget(QWidget *parent/*, RadarInfo *ri*/)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)/*,m_ri(ri)*/
{

    /*
    spokeDrawer = RadarDraw::make_Draw(m_ri,0);

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

void RadarWidget::resizeGL(int width, int height)
{
    setupViewport(width, height);
}

RadarWidget::~RadarWidget()
{
}

void RadarWidget::initializeGL()
{
    glEnable(GL_MULTISAMPLE);
}

void RadarWidget::setupViewport(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef QT_OPENGL_ES
    glOrthof(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#else
    glOrtho(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#endif
    glMatrixMode(GL_MODELVIEW);
}

