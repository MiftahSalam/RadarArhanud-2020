#include <QOpenGLFunctions_3_0>
#include <stdlib.h>
#include <math.h>

#include "radarwidget.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

RadarWidget::RadarWidget(QWidget *parent, int map_zoom_level)
    : QOpenGLWidget(parent),m_range_meters(0), mapZoomLevel(map_zoom_level), m_range_pixel(0), curRadarScaled(1.0)
{
    m_ri = new RI(this);
    spokeDrawer = RD::make_Draw(m_ri,0);
    arpa = m_ri->m_arpa;

    connect(m_ri,SIGNAL(signal_plotRadarSpoke(int,int,u_int8_t*,size_t)),
            this,SLOT(trigger_DrawSpoke(int,int,u_int8_t*,size_t)));
    connect(m_ri,SIGNAL(signal_range_change(int)),
            this,SLOT(trigger_RangeChange(int)));
    /*


    timer  = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeOut()));
    timer->start(100);
    */

    setAutoFillBackground(false);

    /*
    old_motion_mode = radar_settings.headingUp;
    curRange = 0;
    cur_arpa_id_count = 0;
    cur_arpa_number = 0;
    arpa_measure_time = QDateTime::currentMSecsSinceEpoch();
    */

    arpaList.clear();

    timerId = startTimer(1000);
}

void RadarWidget::trigger_reqCreateArpa(QPointF position)
{
    qDebug()<<Q_FUNC_INFO<<position;

    Position arpa_pos;
    arpa_pos.lat = position.y();
    arpa_pos.lon = position.x();

    arpa->AcquireNewMARPATarget(arpa_pos);
}

void RadarWidget::trigger_RangeChange(int rng)
{
    qDebug()<<Q_FUNC_INFO<<m_range_meters<<rng;

    arpa->range_meters = rng;
    m_range_meters = rng;
    const QList<int> distanceList = QList<int>()<<5000000<<2000000<<1000000<<1000000<<1000000<<
                                                   100000<<100000<<50000<<50000<<10000<<10000<<
                                                   10000<<1000<<1000<<500<<200<<100<<50<<25;
    int zoom_index;
    int cur_map_scale;
    double line_per_cur_scale;
    double map_meter_per_pixel;
    double radar_meter_per_pixel;

    for(zoom_index = distanceList.size()-1; zoom_index>=0
        ; zoom_index--)
    {
        cur_map_scale = distanceList.at(zoom_index);
        line_per_cur_scale = cur_map_scale / pow(2.0, 18-zoom_index ) / 0.597164;
        map_meter_per_pixel =  ((double)cur_map_scale)/line_per_cur_scale;
        radar_meter_per_pixel =  ((double)m_range_meters)/(double)m_range_pixel;

        if(map_meter_per_pixel >= radar_meter_per_pixel)
//            if(map_meter_per_pixel <= radar_meter_per_pixel)
            break;
    }
    /*
    int cur_map_scale = distanceList.at(zoom_index);
    double line_per_cur_scale = distanceList.at( mapZoomLevel ) / pow(2.0, 18-mapZoomLevel ) / 0.597164;
    double map_meter_per_pixel =  ((double)cur_map_scale)/line_per_cur_scale;
    double radar_meter_per_pixel =  ((double)m_range_meters)/(double)m_range_pixel;
    */
    mapZoomLevel = zoom_index;
    curRadarScaled = radar_meter_per_pixel/map_meter_per_pixel;
//    curRadarScaled *= 2.0;

//    radar_settings.last_scale = m_range_meters;

    emit signal_zoom_change(mapZoomLevel);
    qDebug()<<Q_FUNC_INFO<<zoom_index<<cur_map_scale<<line_per_cur_scale<<map_meter_per_pixel<<radar_meter_per_pixel<<curRadarScaled;

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

    qDebug()<<Q_FUNC_INFO<<arpaList;

    if(arpa->m_number_of_targets > arpaList.size())
    {
//        qDebug()<<"lebih besar";
        QList<int> new_arpa_list;
        for(int i=0; i<arpa->m_number_of_targets; i++)
        {
            qDebug()<<Q_FUNC_INFO<<"arpa->m_target"<<i<<arpa->m_target[i]->m_target_id;
            if(!arpaList.contains(arpa->m_target[i]->m_target_id) /*&& (arpa->m_target[i]->getStatus()>5)*/)
            {
//                qDebug()<<Q_FUNC_INFO<<"arpaList not contain"<<arpa->m_target[i]->m_target_id;
                new_arpa_list.append(arpa->m_target[i]->m_target_id);
                emit signal_arpaChange(true,arpa->m_target[i]->m_target_id);
            }
        }

        for(int i=0; i<new_arpa_list.size(); i++)
        {
            arpaList.append(new_arpa_list.at(i));
        }
    }
    else if(arpa->m_number_of_targets < arpaList.size())
    {
        qDebug()<<"lebih kecil";
        QList<int> del_arpa_list;
        for(int i=0; i<arpaList.size(); i++)
        {
            if(!arpaList.contains(arpa->m_target[i]->m_target_id))
            {
                del_arpa_list.append(arpa->m_target[i]->m_target_id);
                emit signal_arpaChange(false,arpa->m_target[i]->m_target_id);
            }
        }

        for(int i=0; i<del_arpa_list.size(); i++)
        {
            arpaList.removeAll(del_arpa_list.at(i));
        }
    }

    if(arpa->m_number_of_targets > 0)
    {
        arpa->RefreshArpaTargets();

        /*
        qDebug()<<Q_FUNC_INFO<<arpa->m_number_of_targets;
        for(int i=0; i<arpa->m_number_of_targets; i++)
        {
            qDebug()<<Q_FUNC_INFO<<arpa->m_target[i]->m_target_id<<arpa->m_target[i]->m_position.lat<<arpa->m_target[i]->m_position.lon;
        }
    */
    }
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
    f->glScaled(curRadarScaled, curRadarScaled, 1.);
//    f->glScaled(1.0/2.0, 1.0/2.0, 1.);

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
    m_ri->receiveThread->exitReq();
    m_ri->receiveThread->wait(1000000);
}

void RadarWidget::initializeGL()
{
    qDebug()<<Q_FUNC_INFO;

//    glEnable(GL_MULTISAMPLE);
}

void RadarWidget::setupViewport(int width, int height)
{
    QOpenGLFunctions_3_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_0>();
    int side = qMax(width, height);
    if(side != m_range_pixel)
    {
        m_range_pixel = side;
        trigger_RangeChange(m_range_meters);
    }
    //    qDebug()<<Q_FUNC_INFO<<width<<height<<side;

    //    f->glViewport(0, 0, width, height);
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

