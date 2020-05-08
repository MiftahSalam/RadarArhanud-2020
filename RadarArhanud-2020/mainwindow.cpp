#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_ri = new RI(this);
    m_ra = new RA(this,m_ri);

    /*
    old_motion_mode = radar_settings.headingUp;
    curRange = 0;
    cur_arpa_id_count = 0;
    cur_arpa_number = 0;
    arpa_measure_time = QDateTime::currentMSecsSinceEpoch();
    */

    QGLWidget *glw = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    glw->makeCurrent();

    scene = new RadarScene(this,m_ra,m_ri);

    ui->setupUi(this);

    ui->gridLayout->removeWidget(ui->graphicsView);
    ui->gridLayout->removeWidget(ui->frameBottom);
    ui->gridLayout->removeWidget(ui->frameLeft);

    ui->gridLayout->addWidget(ui->graphicsView,0,1,4,10);
    ui->gridLayout->addWidget(ui->frameBottom,4,1,1,10);
    ui->gridLayout->addWidget(ui->frameLeft,0,0,5,1);
    /*
    ui->gridLayout->addWidget(ui->graphicsView,0,0,3,3);
    ui->gridLayout->addWidget(ui->frame,3,0,1,3);
    ui->gridLayout->addWidget(ui->frame_2,0,3,4,1);
    */

    ui->graphicsView->setViewport(glw);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setMouseTracking(true);
    installEventFilter(ui->graphicsView);

    ui->graphicsView->tesCreateItem(); // temporary

//    ui->frameBottom->setRadarInfo(ui->graphicsView->getRadarInfo());

    connect(ui->frameLeft,SIGNAL(signal_exit()),this,SLOT(close()));
    connect(ui->frameLeft,SIGNAL(signal_mapChange(quint8,quint8)),
            ui->graphicsView,SLOT(trigger_mapChange(quint8,quint8)));
    connect(ui->graphicsView,SIGNAL(signal_rangeChange(qreal)),ui->frameLeft,SLOT(trigger_rangeChange(qreal)));

    connect(ui->graphicsView,SIGNAL(signal_reqCreateArpa(QPointF)),
            this,SLOT(trigger_reqCreateArpa(QPointF)));
    connect(ui->graphicsView,SIGNAL(signal_mapChange(QImage)),
            scene,SLOT(trigger_mapChange(QImage)));
    connect(ui->graphicsView,SIGNAL(signal_cursorPosition(qreal,qreal,qreal,qreal)),
            scene,SLOT(trigger_cursorPosition(qreal,qreal,qreal,qreal)));
    connect(m_ri,SIGNAL(signal_range_change(int)),
            this,SLOT(trigger_rangeChange(int)));
    connect(m_ri,SIGNAL(signal_plotRadarSpoke(int,int,u_int8_t*,size_t)),
            this,SLOT(trigger_DrawSpoke(int,int,u_int8_t*,size_t)));
}

void MainWindow::trigger_DrawSpoke(int transparency, int angle, u_int8_t *data, size_t len)
{
    scene->DrawSpoke(transparency,angle,data,len);
    m_ra->RefreshArpaTargets();
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

void MainWindow::trigger_rangeChange(int rng)
{
    qDebug()<<Q_FUNC_INFO<<rng;

    m_ra->range_meters = rng;
    m_range_meters = rng;

    ui->frameLeft->setRangeRings(ui->graphicsView->calculateRangeRing());

    calculateRadarScale();
}

void MainWindow::trigger_reqCreateArpa(QPointF position)
{
    //    qDebug()<<Q_FUNC_INFO<<position;

    Position arpa_pos;
    arpa_pos.lat = position.y();
    arpa_pos.lon = position.x();

    m_ra->AcquireNewMARPATarget(arpa_pos);

    scene->reqNewArpa(true,m_ra->m_target[m_ra->m_number_of_targets-1]);
}

void MainWindow::calculateRadarScale()
{
    int zoom_index;
    int cur_map_scale;
    float line_per_cur_scale;
    float map_meter_per_pixel;
    float radar_meter_per_pixel =  ((double)m_range_meters)/(double)m_range_pixel;

    for(zoom_index = distanceList.size()-1; zoom_index>=0
        ; zoom_index--)
    {
        cur_map_scale = distanceList.at(zoom_index);
        line_per_cur_scale = cur_map_scale / pow(2.0, 18-zoom_index ) / 0.597164;
        map_meter_per_pixel =  ((double)cur_map_scale)/line_per_cur_scale;

        if(map_meter_per_pixel >= radar_meter_per_pixel)
            break;
    }

    ui->graphicsView->setMapZoomLevel(zoom_index);
    float curRadarScaled = radar_meter_per_pixel/map_meter_per_pixel;
    curRadarScaled *= 2.0;
    scene->setRadarScale(curRadarScaled);

    qDebug()<<Q_FUNC_INFO<<"m_range_meters"<<m_range_meters;
    qDebug()<<Q_FUNC_INFO<<"m_range_pixel"<<m_range_pixel;
    qDebug()<<Q_FUNC_INFO<<"zoom_index"<<zoom_index;
    qDebug()<<Q_FUNC_INFO<<"cur_map_scale"<<cur_map_scale;
    qDebug()<<Q_FUNC_INFO<<"line_per_cur_scale"<<line_per_cur_scale;
    qDebug()<<Q_FUNC_INFO<<"map_meter_per_pixel"<<map_meter_per_pixel;
    qDebug()<<Q_FUNC_INFO<<"radar_meter_per_pixel"<<radar_meter_per_pixel;
    qDebug()<<Q_FUNC_INFO<<"curRadarScaled"<<curRadarScaled;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<Q_FUNC_INFO;

    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");

    config.setValue("radar/show_ring",radar_settings.show_rings);
    config.setValue("radar/show_heading_marker",radar_settings.show_heading_marker);
    config.setValue("radar/show_compass",radar_settings.show_compass);

    config.setValue("arpa/create_arpa_by_click",arpa_settings.create_arpa_by_click);
    config.setValue("arpa/show",arpa_settings.show);
    config.setValue("arpa/min_contour_len",arpa_settings.min_contour_length);
    config.setValue("arpa/search_radius1",arpa_settings.search_radius1);
    config.setValue("arpa/search_radius2",arpa_settings.search_radius2);
    config.setValue("arpa/max_target_size",arpa_settings.max_target_size);

    config.setValue("map/show",map_settings.show);
    config.setValue("map/mode",map_settings.mode);

    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
//    qDebug()<<Q_FUNC_INFO<<ui->graphicsView->size();
    m_range_pixel = qMax(ui->graphicsView->width(),ui->graphicsView->height());
    calculateRadarScale();
    ui->frameLeft->setRangeRings(ui->graphicsView->calculateRangeRing());
}

MainWindow::~MainWindow()
{
    delete ui;
}
