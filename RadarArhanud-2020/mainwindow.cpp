#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    auto logging = Log4Qt::Logger::rootLogger()->appender("A2");
    Log4Qt::Layout *layout = static_cast<Log4Qt::Layout*>(logging.data()->layout().data());

    logEvent = new Log4Qt::SignalAppender();
    logEvent->setName(QLatin1String("Sig Appender"));
    logEvent->setLayout(layout);
    logEvent->activateOptions();

    connect(logEvent,SIGNAL(appended(QString)),this,SLOT(trigger_logEvent(QString)));
    Log4Qt::Logger::rootLogger()->addAppender(logEvent);

    m_ri = new RI(this);
    m_ra = new RA(this,m_ri);
    timer = new QTimer(this);
    adsb = NULL;
    adsb_list.clear();

    first = true;

    initADSB();

    first = false;
    /*
    connect(ui->frameAdvanceConnSetting,SIGNAL(signal_ADSBSettingsChanged()),
            this,SLOT(initADSB()));
    */

    cur_arpa_id_count = 0;

    QGLWidget *glw = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    glw->makeCurrent();

    scene = new RadarScene(this,m_ri);

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

    connect(this,SIGNAL(signal_trueLog(QString)),ui->frameLeft,SLOT(trigger_newLog(QString)));
    connect(ui->frameLeft,SIGNAL(signal_exit()),this,SLOT(close()));
    connect(ui->frameLeft,SIGNAL(signal_mapChange(quint8,quint8)),
            ui->graphicsView,SLOT(trigger_mapChange(quint8,quint8)));
    connect(ui->frameLeft,SIGNAL(signal_Standby()),m_ri,SIGNAL(signal_sendStby()));
    connect(ui->frameLeft,SIGNAL(signal_Tx()),m_ri,SIGNAL(signal_sendTx()));
    connect(ui->frameLeft,SIGNAL(signal_radarSettingChange()),
            m_ri,SLOT(trigger_ReqRadarSetting()));
    connect(ui->frameLeft,SIGNAL(signal_req_control(int,int)),
            m_ri,SLOT(trigger_ReqControlChange(int,int)));
    connect(ui->frameLeft,SIGNAL(signal_req_range(int)),this,SLOT(trigger_rangeChange(int)));

    connect(ui->frameBottom,SIGNAL(signal_request_del_track(int)),
            this,SLOT(trigger_ReqDelTrack(int)));
    connect(ui->frameBottom,SIGNAL(signal_request_del_adsb_track(quint32)),
            this,SLOT(trigger_ReqDelAdsb(quint32)));

    connect(ui->graphicsView,SIGNAL(signal_reqCreateArpa(QPointF)),
            this,SLOT(trigger_reqCreateArpa(QPointF)));
    connect(ui->graphicsView,SIGNAL(signal_mapChange(QImage)),
            scene,SLOT(trigger_mapChange(QImage)));
    connect(ui->graphicsView,SIGNAL(signal_cursorPosition(qreal,qreal,qreal,qreal)),
            scene,SLOT(trigger_cursorPosition(qreal,qreal,qreal,qreal)));
    connect(m_ri,SIGNAL(signal_range_change(int)),
            this,SLOT(trigger_rangeChange(int)));
    connect(m_ri,SIGNAL(signal_plotRadarSpoke(int,u_int8_t*,size_t)),
            this,SLOT(trigger_DrawSpoke(int,u_int8_t*,size_t)));
    connect(m_ri,SIGNAL(signal_forceExit()),
            this,SLOT(trigger_forceExit()));
    connect(m_ri,SIGNAL(signal_state_change()),ui->frameLeft,SLOT(trigger_stateChange()));
    connect(m_ri,SIGNAL(signal_updateReport()),ui->frameLeft,SLOT(trigger_reportChange()));

    connect(this,SIGNAL(signal_arpa_target_param(int,double,double,double,double,double,double)),
            ui->frameBottom,SLOT(trigger_arpa_target_param(int,double,double,double,double,double,double)));
    connect(this,SIGNAL(signal_adsb_target_param(quint32,double,double,double,double,double,double,double,QString,QString)),
            ui->frameBottom,SLOT(trigger_adsb_target_update(quint32,double,double,double,double,double,double,double,QString,QString)));
    connect(this,SIGNAL(signal_reqRangeChange(int)),m_ri,SLOT(trigger_ReqRangeChange(int)));
    connect(timer,SIGNAL(timeout()),this,SLOT(timeOut()));
    connect(timer,SIGNAL(timeout()),ui->frameLeft,SLOT(trigger_stateChange()));

    int g;
    for (g = 0; g < ARRAY_SIZE(g_ranges_metric); g++)
    {
        if (QString(g_ranges_metric[g].name )== "1.5 NM")
        {
            trigger_rangeChange(g_ranges_metric[g].meters);
            break;
        }
    }

    timer->start(1000);
}

void MainWindow::trigger_DrawSpoke(int angle, u_int8_t *data, size_t len)
{
    scene->DrawSpoke(angle,data,len);
    m_ra->RefreshArpaTargets();
}

/*
*/
void MainWindow::trigger_ReqDelTrack(int id)
{
    if(id>-10)
    {
        for(int i=0;i<m_ra->m_number_of_targets;i++)
            if(m_ra->m_target[i]->m_target_id == id)
                m_ra->m_target[i]->SetStatusLost();
    }
    else
        m_ra->DeleteAllTargets();
}
/**/
void MainWindow::timeOut()
{
//    qDebug()<<Q_FUNC_INFO<<"adsb_list"<<adsb_list;

    m_ra->RefreshArpaTargets();

    if(m_ra->m_number_of_targets > 0)
    {
        int num_limit = 5;
        while ((cur_arpa_id_count < m_ra->m_number_of_targets) && num_limit > 0)
        {
            if(m_ra->m_target[cur_arpa_id_count]->m_target_id > 0)
            {
                double dif_lat = deg2rad(m_ra->m_target[cur_arpa_id_count]->m_position.lat);
                double dif_lon = (deg2rad(m_ra->m_target[cur_arpa_id_count]->m_position.lon)
                                  - deg2rad(currentOwnShipLon))
                        *cos(deg2rad((currentOwnShipLat+m_ra->m_target[cur_arpa_id_count]->m_position.lat)/2.));
                double R = 6371.;

                dif_lat =  dif_lat - (deg2rad(currentOwnShipLat));

                double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
                qreal bearing = atan2(dif_lon,dif_lat)*180./M_PI;

                while(bearing < 0.0)
                {
                    bearing += 360.0;
                }

//                qDebug()<<Q_FUNC_INFO<<m_ra->m_target[cur_arpa_id_count]->m_target_id<<km<<bearing;

                emit signal_arpa_target_param(m_ra->m_target[cur_arpa_id_count]->m_target_id,
                                              km,
                                              bearing,
                                              m_ra->m_target[cur_arpa_id_count]->m_position.lat,
                                              m_ra->m_target[cur_arpa_id_count]->m_position.lon,
                                              m_ra->m_target[cur_arpa_id_count]->m_speed_kn,
                                              m_ra->m_target[cur_arpa_id_count]->m_course
                                              );
            }
            cur_arpa_id_count++;
            num_limit--;
        }
        if(cur_arpa_id_count >= m_ra->m_number_of_targets)
            cur_arpa_id_count = 0;
    }

    QHash<int,AdsbArhnd::ADSBTargetData*> buf_adsb = adsb->getADSB().getTargets();

    /*
    ui->frameADSB->updateADSBList(buf_adsb);
    ui->frameADSB->updateADSBStatus(adsb->getCurrentSensorStatus(),cms->getCurrentError().isEmpty());
    */

    /*
    if(curShowADSBStream ^ showADSBStream)
    {
        qDebug()<<Q_FUNC_INFO<<curShowADSBStream<<showADSBStream;
        curShowADSBStream = showADSBStream;

        if(curShowADSBStream)
            connect(adsb,SIGNAL(signal_sendStreamData(QByteArray)),ui->frameAdvanceStream,SLOT(setADSBStreamData(QByteArray)));
        else
            disconnect(adsb,SIGNAL(signal_sendStreamData(QByteArray)),ui->frameAdvanceStream,SLOT(setADSBStreamData(QByteArray)));
    }
    */
}

void MainWindow::initADSB()
{
    AdsbArhnd::StreamSettings adsbSettingIn;

    adsbSettingIn.config = adsb_settings.config;
    adsbSettingIn.mode = AdsbArhnd::In;
    adsbSettingIn.type = (AdsbArhnd::StreamType)adsb_settings.type;

    if(adsb)
        adsb->setInSettings(adsbSettingIn);
    else
        adsb = new AdsbArhnd::ADSBStream(0,adsbSettingIn);

    /*
    */
    if(adsb_settings.show_track)
    {
        connect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),this,SLOT(trigger_reqUpdateADSB(QByteArray)));
//        connect(adsb,SIGNAL(signal_sendStreamData(QByteArray)),ui->frameAdvanceStream,SLOT(setADSBStreamData(QByteArray)));
//        connect(adsb,SIGNAL(signal_sendStreamData(QByteArray)),ui->frameAdvanceStream,SLOT(setADSBStreamData(QByteArray)));
    }
    else
    {
        disconnect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),this,SLOT(trigger_reqUpdateADSB(QByteArray)));
//        disconnect(adsb,SIGNAL(signal_sendStreamData(QByteArray)),ui->frameAdvanceStream,SLOT(setADSBStreamData(QByteArray)));
    }

    if(!adsb->getCurrentInputError().isEmpty() && (!first))
    {
        QMessageBox::warning(this,"Warning",QString("ADSB input error : ").append(adsb->getCurrentInputError()),QMessageBox::Ok);
    }
}


void MainWindow::trigger_forceExit()
{
    qDebug()<<Q_FUNC_INFO;
    sleep(1);
    close();
}

void MainWindow::trigger_rangeChange(int rng)
{
    qDebug()<<Q_FUNC_INFO<<rng;

    m_ra->range_meters = rng;
    m_range_meters = rng;
//    m_range_meters = 400; //tes

    if(state_radar == RADAR_TRANSMIT)
    {
        ui->frameLeft->setRangeText(rng);
        emit signal_reqRangeChange(rng);
    }

    calculateRadarScale();
    ui->frameLeft->setRangeRings(ui->graphicsView->calculateRangeRing());
}

void MainWindow::trigger_ReqDelAdsb(quint32 icao)
{
    qDebug()<<Q_FUNC_INFO<<"icao"<<icao;
    adsb_list.remove(icao);
}

void MainWindow::trigger_reqUpdateADSB(QByteArray data_in)
{
    QDataStream stream_in(&data_in,QIODevice::ReadOnly);

    stream_in.setByteOrder(QDataStream::LittleEndian);
    stream_in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    float lat,lon,alt,cog,sog;
    quint32 icao;
    QString str_call_sign,str_country;
    char chr_callsign[10],chr_country[10];
    int call_sign_index_sep = 0;
    int country_index_sep = 0;

    stream_in>>icao;
    stream_in.readRawData(chr_callsign,10);
    stream_in>>lat;
    stream_in>>lon;
    stream_in>>alt;
    stream_in>>sog;
    stream_in>>cog;
    stream_in.readRawData(chr_country,10);

    str_call_sign = QString::fromLocal8Bit((const char*)&chr_callsign,10);
    str_country = QString::fromLocal8Bit((const char*)&chr_country,10);
    call_sign_index_sep = str_call_sign.indexOf("@");
    country_index_sep = str_country.indexOf("@");
    str_call_sign.remove(call_sign_index_sep,10-call_sign_index_sep);
    str_country.remove(country_index_sep,10-country_index_sep);

    double dif_lat = deg2rad(lat);
    double dif_lon = (deg2rad(lon)-deg2rad(currentOwnShipLon))*cos(deg2rad((currentOwnShipLat+lat)/2.));
    double R = 6371.;

    dif_lat =  dif_lat - (deg2rad(currentOwnShipLat));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
    qreal bearing = atan2(dif_lon,dif_lat)*180./M_PI;

    while(bearing < 0.0)
    {
        bearing += 360.0;
    }

    if(adsb)
    {
        if(!adsb_list.contains(icao))
        {
//            qDebug()<<Q_FUNC_INFO<<"curTarget icao"<<icao;
            adsb_list.insert(icao);
            scene->reqNewADSB(adsb->getADSB().getTarget(icao));
        }
        /*
        QSetIterator<quint32> i(adsb_list);
        while (i.hasNext())
        {
            quint32 cur_icao = i.next();
            qDebug()<<Q_FUNC_INFO<<"curTarget"<<cur_icao<<adsb->getADSB().getTarget(cur_icao);
        }
        */
    }
//    qDebug()<<endl;
    emit signal_adsb_target_param(icao,km,bearing,lat,lon,sog,cog,alt,str_call_sign,str_country);

    /*
    qDebug()<<Q_FUNC_INFO<<"curTarget icao"<<QString::number((qint32)icao,16);
    qDebug()<<Q_FUNC_INFO<<"curTarget->CallSign chr"<<chr_callsign;
    qDebug()<<Q_FUNC_INFO<<"curTarget->CallSign str"<<str_call_sign;
    qDebug()<<Q_FUNC_INFO<<"curTarget->Lat"<<lat;
    qDebug()<<Q_FUNC_INFO<<"curTarget->Lon"<<lon;
    qDebug()<<Q_FUNC_INFO<<"curTarget->Rng"<<km;
    qDebug()<<Q_FUNC_INFO<<"curTarget->bearing"<<bearing;
    qDebug()<<Q_FUNC_INFO<<"curTarget->alt"<<alt;
    qDebug()<<Q_FUNC_INFO<<"curTarget->COG"<<cog;
    qDebug()<<Q_FUNC_INFO<<"curTarget->SOG"<<sog;
    qDebug()<<Q_FUNC_INFO<<"curTarget->contry chr"<<chr_country;
    qDebug()<<Q_FUNC_INFO<<"curTarget->contry str"<<str_country;
    */
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
    radar_meter_per_pixel *= 1.0;

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
    Log4Qt::Logger::rootLogger()->trace()<<Q_FUNC_INFO;

    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");

    config.setValue("radar/show_ring",radar_settings.show_rings);
    config.setValue("radar/show_heading_marker",radar_settings.show_heading_marker);
    config.setValue("radar/show_compass",radar_settings.show_compass);
    config.setValue("radar/ip_data",radar_settings.ip_data);
    config.setValue("radar/ip_report",radar_settings.ip_report);
    config.setValue("radar/ip_command",radar_settings.ip_command);
    config.setValue("radar/port_command",radar_settings.port_command);
    config.setValue("radar/port_report",radar_settings.port_report);
    config.setValue("radar/port_data",radar_settings.port_data);

    config.setValue("trail/enable",trail_settings.enable);
    config.setValue("trail/mode",trail_settings.trail);

    config.setValue("iff/ip",iff_settings.ip);
    config.setValue("iff/port",iff_settings.port);
    config.setValue("iff/show_track",iff_settings.show_track);

    config.setValue("adsb/config",adsb_settings.config);
    config.setValue("adsb/type",(int)adsb_settings.type);
    config.setValue("adsb/show_track",adsb_settings.show_track);

    config.setValue("map/show",map_settings.show);
    config.setValue("map/mode",map_settings.mode);

    config.setValue("nav_sensor/heading",currentHeading);
    config.setValue("nav_sensor/latitude",currentOwnShipLat);
    config.setValue("nav_sensor/longitude",currentOwnShipLon);

    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    Log4Qt::Logger::rootLogger()->trace()<<Q_FUNC_INFO<<ui->graphicsView->size().width()<<ui->graphicsView->size().height();
    m_range_pixel = qMax(ui->graphicsView->width(),ui->graphicsView->height());
    calculateRadarScale();
    ui->frameLeft->setRangeRings(ui->graphicsView->calculateRangeRing());
}

void MainWindow::trigger_logEvent(QString msg)
{
    int nxt_idx_space = msg.indexOf(" ",20);
    QString type_section = msg.mid(20,nxt_idx_space-20);

    if(type_section == "DEBUG" || type_section == "TRACE")
        return;

    emit signal_trueLog(msg);

}

MainWindow::~MainWindow()
{
    delete ui;
}
