#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QSettings>
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

    m_ri = new RadarEngineARND::RadarEngine(this,0);
    m_ri1 = new RadarEngineARND::RadarEngine(this,1);
    timer = new QTimer(this);
    adsb = NULL;
    adsb_list.clear();

    //test
    StreamArhnd::StreamSettings iffSettingIn, iffSettingOut;
    iffSettingIn.config = "127.0.0.1;8090";
    iffSettingIn.mode = StreamArhnd::In;
    iffSettingIn.type = (StreamArhnd::StreamType)1; //tcp
    iffSettingOut.config = "192.168.1.7;23000";
    iffSettingOut.mode = StreamArhnd::InOut;
    iffSettingOut.type = (StreamArhnd::StreamType)1; //tcp
//    iffSettingOut.config = "127.0.0.1;8070";
//    iffSettingOut.mode = StreamArhnd::InOut;
//    iffSettingOut.type = (StreamArhnd::StreamType)1; //tcp
//    iffSettingOut.config = "/dev/ttyUSB0;38400";
//    iffSettingOut.mode = StreamArhnd::InOut;
//    iffSettingOut.type = (StreamArhnd::StreamType)0; //serial
    iff = IFFArhnd::IFFService::getIntance(iffSettingIn, iffSettingOut);

    cur_arpa_id_count = 0;
    curState = state_radar;

    QGLWidget *glw = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    glw->makeCurrent();

    scene = new RadarScene(this,m_ri,m_ri1);
    m_tm = new TrackManager(m_ri, m_ri1, scene);

    ui->setupUi(this);

    trackDialog = new DialogSelectedTrack(this);
    trackDialog->hide();

    ui->gridLayout->removeWidget(ui->graphicsView);
    ui->gridLayout->removeWidget(ui->frameBottom);
    ui->gridLayout->removeWidget(ui->frameLeft);

    /*
    ui->gridLayout->addWidget(ui->graphicsView,0,1,8,10);
    ui->gridLayout->addWidget(ui->frameLeft,0,0,8,1);
    ui->frameBottom->hide();
    */

    ui->gridLayout->addWidget(ui->graphicsView,0,1,6,10);
    ui->gridLayout->addWidget(ui->frameBottom,6,1,1,10);
    ui->gridLayout->addWidget(ui->frameLeft,0,0,7,1);

    ui->graphicsView->setViewport(glw);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setMouseTracking(true);
    installEventFilter(ui->graphicsView);

    ui->graphicsView->tesCreateItem(); // temporary

    first = true;
    initADSB();
    first = false;

    connect(this,SIGNAL(signal_trueLog(QString)),ui->frameLeft,SLOT(trigger_newLog(QString)));
    connect(ui->frameLeft,SIGNAL(signal_exit()),this,SLOT(close()));
    connect(ui->frameLeft,SIGNAL(signal_changeOpMode(bool)),this,SLOT(trigger_opModeChange(bool)));
    connect(ui->frameLeft,SIGNAL(signal_mapChange(quint8,quint8)),
            ui->graphicsView,SLOT(trigger_mapChange(quint8,quint8)));
    connect(ui->frameLeft,SIGNAL(signal_Standby()),m_ri,SIGNAL(signal_sendStby()));
    connect(ui->frameLeft,SIGNAL(signal_Standby()),m_ri1,SIGNAL(signal_sendStby()));
    connect(ui->frameLeft,SIGNAL(signal_Tx()),m_ri,SIGNAL(signal_sendTx()));
    connect(ui->frameLeft,SIGNAL(signal_Tx()),m_ri1,SIGNAL(signal_sendTx()));
    connect(ui->frameLeft,SIGNAL(signal_radarSettingChange()),
            m_ri,SLOT(trigger_ReqRadarSetting()));
    connect(ui->frameLeft,SIGNAL(signal_radarSettingChange()),
            m_ri1,SLOT(trigger_ReqRadarSetting()));
    connect(ui->frameLeft,SIGNAL(signal_adsbSettingChange()),
            this,SLOT(initADSB()));
    connect(ui->frameLeft,SIGNAL(signal_req_control(int,int)),
            m_ri,SLOT(trigger_ReqControlChange(int,int)));
    connect(ui->frameLeft,SIGNAL(signal_req_control(int,int)),
            m_ri1,SLOT(trigger_ReqControlChange(int,int)));
    connect(ui->frameLeft,SIGNAL(signal_req_range()),this,SLOT(trigger_rangeChange()));
    connect(ui->frameLeft,SIGNAL(signal_clearTrail()),m_ri,SLOT(trigger_clearTrail()));
    connect(ui->frameLeft,SIGNAL(signal_clearTrail()),m_ri1,SLOT(trigger_clearTrail()));
    connect(scene,&RadarScene::signal_zero_detect,ui->frameLeft,&FrameLeft::trigger_changeAntene);

    connect(ui->frameBottom,SIGNAL(signal_request_del_track(int)),
            this,SLOT(trigger_ReqDelTrack(int)));
    connect(ui->frameBottom,SIGNAL(signal_request_del_adsb_track(quint32)),
            this,SLOT(trigger_ReqDelAdsb(quint32)));
    connect(ui->frameBottom,&FrameBottom::signal_update_track_num,this,&MainWindow::trigger_updateTrackNumber);
    connect(ui->frameBottom,&FrameBottom::signal_target_select_update,
            trackDialog,&DialogSelectedTrack::trigger_target_select_update);

    connect(iff,&IFFArhnd::IFFService::signal_update_target_data,this,&MainWindow::trigger_reqUpdateIff);
    connect(iff,&IFFArhnd::IFFService::signal_ModeAResponse,this,&MainWindow::trigger_reqUpdateTrackIdentity);

    connect(ui->graphicsView,SIGNAL(signal_reqCreateArpa(QPointF)),
            this,SLOT(trigger_reqCreateArpa(QPointF)));
    connect(ui->graphicsView,SIGNAL(signal_mapChange(QImage)),
            scene,SLOT(trigger_mapChange(QImage)));
    connect(ui->graphicsView,SIGNAL(signal_cursorPosition(qreal,qreal,qreal,qreal)),
            scene,SLOT(trigger_cursorPosition(qreal,qreal,qreal,qreal)));
//    connect(ui->graphicsView,&RadarGraphicView::signal_positionChange,this,&MainWindow::trigger_positionChange);
    connect(ui->graphicsView,SIGNAL(signal_selectedChange(int,bool)),
            ui->frameBottom,SLOT(trigger_target_selected(int,bool)));

    connect(m_ri,SIGNAL(signal_range_change(int,int)),
            this,SLOT(trigger_radarFeedbackRangeChange(int,int)));
    connect(m_ri1,SIGNAL(signal_range_change(int,int)),
            this,SLOT(trigger_radarFeedbackRangeChange(int,int)));
    connect(m_ri,SIGNAL(signal_plotRadarSpoke(int,u_int8_t*,size_t)),
            this,SLOT(trigger_DrawSpoke(int,u_int8_t*,size_t)));
    connect(m_ri1,SIGNAL(signal_plotRadarSpoke(int,u_int8_t*,size_t)),
            this,SLOT(trigger_DrawSpoke1(int,u_int8_t*,size_t)));
    connect(m_ri,SIGNAL(signal_forceExit()),
            this,SLOT(trigger_forceExit()));
    connect(m_ri,SIGNAL(signal_state_change()),ui->frameLeft,SLOT(trigger_stateChange()));
    connect(m_ri1,SIGNAL(signal_state_change()),ui->frameLeft,SLOT(trigger_stateChange()));
//    connect(m_ri1,SIGNAL(signal_updateReport()),ui->frameLeft,SLOT(trigger_reportChange()));

    connect(m_tm,SIGNAL(signal_target_param(quint32,double,double,double,double,double,double,double,QString,QString,bool,quint8)),
            this,SIGNAL(signal_target_param(quint32,double,double,double,double,double,double,double,QString,QString,bool,quint8)));
    connect(this,SIGNAL(signal_target_param(quint32,double,double,double,double,double,double,double,QString,QString,bool,quint8)),
            ui->frameBottom,SLOT(trigger_target_update(quint32,double,double,double,double,double,double,double,QString,QString,bool,quint8)));
    connect(this,SIGNAL(signal_reqRangeChange(int,int)),m_ri,SLOT(trigger_ReqRangeChange(int,int)));
    connect(this,SIGNAL(signal_reqRangeChange(int,int)),m_ri1,SLOT(trigger_ReqRangeChange(int,int)));
    connect(timer,SIGNAL(timeout()),this,SLOT(timeOut()));
    connect(timer,SIGNAL(timeout()),ui->frameLeft,SLOT(trigger_stateChange()));

    trigger_rangeChange();
    timer->start(1000);

}

//void MainWindow::trigger_positionChange()
//{
//    if(radar_settings.op_mode)
//        ui->frameLeft->setRangeRings(10.);
//    else
//        ui->frameLeft->setRangeRings(ui->graphicsView->calculateRangeRing());
//}

void MainWindow::trigger_DrawSpoke(int angle, u_int8_t *data, size_t len)
{
//    qDebug()<<Q_FUNC_INFO;
    scene->DrawSpoke(angle,data,len);
    m_tm->refreshTarget();
//    for (int i = 0; i < ANTENE_COUNT; ++i) {
//        m_ri->radarArpa[i]->RefreshArpaTargets();
//    }
}

void MainWindow::trigger_DrawSpoke1(int angle, u_int8_t *data, size_t len)
{
//    qDebug()<<Q_FUNC_INFO;
    scene->DrawSpoke1(angle,data,len);
    m_tm->refreshTarget1();
//    for (int i = 0; i < ANTENE_COUNT; ++i) {
//        m_ri1->radarArpa[i]->RefreshArpaTargets();
//    }
}

void MainWindow::trigger_updateTrackNumber(int id, int number)
{
    if(id <= 100)
    {
//        m_tm->updateTrackNumber(id,number);
//        for (int h = 0; h < ANTENE_COUNT; ++h)
//        {
//            for(int i=0;i<m_ri->radarArpa[h]->m_number_of_targets;i++)
//            {
//                if(m_ri->radarArpa[h]->m_target[i]->m_target_id == (int)id)
//                {
//                    m_ri->radarArpa[h]->m_target[i]->m_target_number = number;
//                }
//            }
//        }
    }
    else
    {
        adsb->getADSB().setTargetNumber(id,number);
    }
}

/*
*/
void MainWindow::trigger_ReqDelTrack(int id)
{
    qDebug()<<Q_FUNC_INFO<<(int)id;
    m_tm->reqDelTrack(id);
//    if(id>-10)
//    {
//        for (int h = 0; h < ANTENE_COUNT; ++h)
//        {
//            for(int i=0;i<m_ri->radarArpa[h]->m_number_of_targets;i++)
//            {
//                if(m_ri->radarArpa[h]->m_target[i]->m_target_id == (int)id)
//                {
//                    m_ri->radarArpa[h]->m_target[i]->SetStatusLost();
//                    qDebug()<<Q_FUNC_INFO<<m_ri->radarArpa[h]->m_target[i]->m_target_id<<(int)id;
//                }
//            }
//        }
//    }
//    else
//    {
//        for (int i = 0; i < ANTENE_COUNT; ++i) {
//            m_ri->radarArpa[i]->DeleteAllTargets();
//        }
//    }
}

/**/
void MainWindow::timeOut()
{
//    qDebug()<<Q_FUNC_INFO<<"adsb_list"<<adsb_list;

//    filter.wakingup_time++; //tes

    m_tm->updateTracks();
//    for (int h = 0; h < ANTENE_COUNT; ++h)
//    {
//        m_ri->radarArpa[h]->RefreshArpaTargets();
//        m_ri1->radarArpa[h]->RefreshArpaTargets();

//        if(m_ri->radarArpa[h]->m_number_of_targets > 0)
//        {
//            int num_limit = 5;
//            while ((cur_arpa_id_count < m_ri->radarArpa[h]->m_number_of_targets) && num_limit > 0)
//            {
//                if(m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_target_id > 0)
//                {
//                    emit signal_target_param(m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_target_id,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.rng,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.brn,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.lat,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.lon,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_speed_kn,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_course,
//                                                  m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->m_position.alt,
//                                                  "-","-",m_ri->radarArpa[h]->m_target[cur_arpa_id_count]->selected
//                                                  );
//                }
//                cur_arpa_id_count++;
//                num_limit--;
//            }
//            if(cur_arpa_id_count >= m_ri->radarArpa[h]->m_number_of_targets)
//                cur_arpa_id_count = 0;
//        }
//    }

//    RadarState cur_radar_state = decideRadarState(state_radar, state_radar1);

//    if(cur_radar_state == RADAR_TRANSMIT)
    if(state_radar == RADAR_TRANSMIT || state_radar1 == RADAR_TRANSMIT)
    {
        adjustRadarRange();
    }
//    qDebug()<<Q_FUNC_INFO<<"m_range_meters"<<m_range_meters<<"m_range_meters"<<m_radar_range;

    ui->frameLeft->setAdsbStatus((int)adsb->getCurrentSensorStatus());
    ui->frameLeft->updateIffStatus();
    ui->frameLeft->setNavStatus(ui->frameBottom->getNavStatus());
    ui->frameLeft->updateRadarStatus();

    if(adsb)
    {
        adsb->setLatLon(currentOwnShipLat,currentOwnShipLon);

//        state_radar = RADAR_TRANSMIT; //tes
//        if(curState != cur_radar_state)
        if(curState != state_radar)
        {
            if(state_radar != RADAR_TRANSMIT)
//            if(cur_radar_state != RADAR_TRANSMIT)
            {
                ui->graphicsView->showAdsb(false);
                disconnect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),this,SLOT(trigger_reqUpdateADSB(QByteArray)));
            }
            else if(state_radar == RADAR_TRANSMIT)
//            else if(cur_radar_state == RADAR_TRANSMIT)
            {
                ui->graphicsView->showAdsb(adsb_settings.show_track);
                connect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),
                        this,SLOT(trigger_reqUpdateADSB(QByteArray)),Qt::UniqueConnection);
            }
            curState = state_radar;
//            curState = cur_radar_state;
        }
    }
}

void MainWindow::adjustRadarRange()
{
    qDebug()<<Q_FUNC_INFO<<"state_radar == RADAR_TRANSMIT"
           <<"m_range_meters"<<m_range_meters<<"m_radar_range"<<m_radar_range
          <<"100000./1.5"<<100000./1.5;
    if(m_range_meters < 100000.)
    {
        double cur_radar_scale = m_radar_range/m_range_meters;
        qDebug()<<Q_FUNC_INFO<<"m_range_meters < 100000."<<cur_radar_scale<<m_radar_range<<m_range_meters;

        if(fabs(m_range_meters-m_radar_range) > 10)
        {
            m_range_to_send =  m_radar_range/cur_radar_scale;
            m_range_to_send /= 1.5;
            emit signal_reqRangeChange(0,(int)m_range_to_send);
            qDebug()<<Q_FUNC_INFO<<"beda range detek"<<m_range_to_send;
        }
    }
    else if((m_range_meters > 100000.) && (m_radar_range < 80000.))
    {
        m_range_to_send = 100000./1.5;
        emit signal_reqRangeChange(0,(int)m_range_to_send);
        qDebug()<<Q_FUNC_INFO<<"(m_range_meters > 100000.) && (m_radar_range < 80000.)"<<m_range_to_send;
    }

    if(fabs(m_radar_range1-m_radar_range) > 10) emit signal_reqRangeChange(1,(int)m_range_to_send);

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
    if(adsb_settings.show_track)
        connect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),this,SLOT(trigger_reqUpdateADSB(QByteArray)));
    else
        disconnect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),this,SLOT(trigger_reqUpdateADSB(QByteArray)));
    */

//    RadarState cur_radar_state = decideRadarState(state_radar, state_radar1);

//    if(cur_radar_state != RADAR_TRANSMIT)
        if(state_radar != RADAR_TRANSMIT)
    {
        adsb_list.clear();
        ui->graphicsView->showAdsb(false);
        disconnect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),this,SLOT(trigger_reqUpdateADSB(QByteArray)));
    }
//    else if(cur_radar_state == RADAR_TRANSMIT)
        else if(state_radar == RADAR_TRANSMIT)
    {
        ui->graphicsView->showAdsb(adsb_settings.show_track);
        connect(adsb,SIGNAL(signal_updateTargetData(QByteArray)),
                this,SLOT(trigger_reqUpdateADSB(QByteArray)),Qt::UniqueConnection);
    }

    ui->graphicsView->showAdsb(adsb_settings.show_track);
}


void MainWindow::trigger_forceExit()
{
    qDebug()<<Q_FUNC_INFO;
    sleep(1);
    close();
}

void MainWindow::trigger_reqUpdateTrackIdentity(QString data)
{
    QStringList squawk_list = data.split(",",QString::SkipEmptyParts);

    foreach (QString squawk, squawk_list) {
        if(adsb) adsb->setTargetIdentityFromIFF(squawk,1); //friend
    }

}
void MainWindow::trigger_reqUpdateIff(IFFArhnd::IFFTargetData track)
{
    /*
    qDebug()<<Q_FUNC_INFO<<"icao"<<track.icao;
    qDebug()<<Q_FUNC_INFO<<"latitude"<<track.lat;
    qDebug()<<Q_FUNC_INFO<<"longitude"<<track.lon;
    qDebug()<<Q_FUNC_INFO<<"altitude"<<track.alt;
    qDebug()<<Q_FUNC_INFO<<"speed"<<track.speed;
    qDebug()<<Q_FUNC_INFO<<"course"<<track.course;
    qDebug()<<Q_FUNC_INFO<<"rng"<<track.rng;
    qDebug()<<Q_FUNC_INFO<<"brn"<<track.brn;
    qDebug()<<Q_FUNC_INFO<<"vertical_rate"<<track.vertical_rate;
    qDebug()<<Q_FUNC_INFO<<"time_stamp"<<track.time_stamp;
    qDebug()<<Q_FUNC_INFO<<"lat_valid"<<track.lat_valid;
    qDebug()<<Q_FUNC_INFO<<"lon_valid"<<track.lon_valid;
    qDebug()<<Q_FUNC_INFO<<"alt_valid"<<track.alt_valid;
    qDebug()<<Q_FUNC_INFO<<"speed_valid"<<track.speed_valid;
    qDebug()<<Q_FUNC_INFO<<"course_valid"<<track.course_valid;
    qDebug()<<Q_FUNC_INFO<<"vertical_rate_valid"<<track.vertical_rate_valid;
    */
    qDebug()<<Q_FUNC_INFO<<"identity"<<track.identity;

    AdsbArhnd::ADSBTargetData target;

    target.icao = track.icao;
    target.lat = track.lat;
    target.lon = track.lon;
    target.rng = track.rng;
    target.brn = track.brn;
    target.alt = track.alt;
    target.speed = track.speed;
    target.course = track.course;
    target.vertical_rate = track.vertical_rate;
    target.lat_valid = track.lat_valid;
    target.lon_valid = track.lon_valid;
    target.alt_valid = track.alt_valid;
    target.speed_valid = track.speed_valid;
    target.course_valid = track.course_valid;
    target.vertical_rate_valid = track.vertical_rate_valid;
    target.selected = track.selected;
    target.ground = track.ground;
    target.identity = track.identity;
    target.time_stamp = track.time_stamp;

    if(adsb) adsb->setTargetFromIFF(target);
//    if(adsb_list.contains(track.icao))
//    {
//        if(adsb) adsb->getADSB().getTarget(track.icao)->identity = track.identity;
//    }
//    else
//    {
//        qDebug()<<Q_FUNC_INFO<<"new iff target. icao"<<icao;
//        adsb_list.insert(icao);
//        scene->reqNewADSB(adsb->getADSB().getTarget(icao),adsb_settings.show_track);
//    }

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
    quint8 identity;
    QString str_call_sign,str_country;
    char chr_callsign[10],chr_country[10];
    int call_sign_index_sep = 0;
    int country_index_sep = 0;
    bool selected;

    stream_in>>icao;
    stream_in.readRawData(chr_callsign,10);
    stream_in>>lat;
    stream_in>>lon;
    stream_in>>alt;
    stream_in>>sog;
    stream_in>>cog;
    stream_in.readRawData(chr_country,10);
    stream_in>>selected;
    stream_in>>identity;

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
            qDebug()<<Q_FUNC_INFO<<"new curTarget icao"<<icao;
            adsb_list.insert(icao);
            scene->reqNewADSB(adsb->getADSB().getTarget(icao),adsb_settings.show_track);
        }
        /*
        QSetIterator<quint32> i(adsb_list);
        while (i.hasNext())
        {
            quint32 cur_icao = i.next();
            qDebug()<<Q_FUNC_INFO<<"curTarget"<<cur_icao<<adsb->getADSB().getTarget(cur_icao);
        }
        */
//        qDebug()<<Q_FUNC_INFO<<"curTarget  ptr"<<QString::number((qint32)icao,16)<<adsb->getADSB().getTarget(icao);
//        qDebug()<<Q_FUNC_INFO<<"curTarget  rng"<<adsb->getADSB().getTarget(icao)->rng;
//        qDebug()<<Q_FUNC_INFO<<"curTarget  brn"<<adsb->getADSB().getTarget(icao)->brn;
//        qDebug()<<Q_FUNC_INFO<<"curTarget  alt"<<adsb->getADSB().getTarget(icao)->alt;
//        qDebug()<<Q_FUNC_INFO<<"curTarget  speed"<<adsb->getADSB().getTarget(icao)->speed;
//        qDebug()<<Q_FUNC_INFO<<"curTarget  course"<<adsb->getADSB().getTarget(icao)->course;
    }
//    if(km<60.)
    emit signal_target_param(icao,km+0.2,bearing+1.1,lat,lon,sog+1.3,cog+1.3,alt+(150.*0.3048),str_call_sign,str_country,selected,identity);


        /*
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
    qDebug()<<Q_FUNC_INFO<<"curTarget->selected"<<selected;
    */
}


void MainWindow::trigger_reqCreateArpa(QPointF position)
{
    //    qDebug()<<Q_FUNC_INFO<<position;

    m_tm->reqCreateArpa(position);
//    Position arpa_pos;
//    arpa_pos.lat = position.y();
//    arpa_pos.lon = position.x();

//    for (int i = 0; i < ANTENE_COUNT; ++i) {
//        m_ri->radarArpa[i]->AcquireNewMARPATarget(arpa_pos);
//        scene->reqNewArpa(true,true,m_ri->radarArpa[i]->m_target[m_ri->radarArpa[i]->m_number_of_targets-1]);
//    }
}

void MainWindow::trigger_radarFeedbackRangeChange(int rId, int rng) //test again later
{
    qDebug()<<Q_FUNC_INFO<<"rId"<<rId<<"rng"<<rng;
    if(!rId)
    {
        m_radar_range = 1.5*(double)rng;
        calculateRadarScale();
    }
}

void MainWindow::trigger_rangeChange()
{
    qDebug()<<Q_FUNC_INFO;

    calculateRadarScale();
    if(state_radar == RADAR_TRANSMIT || state_radar1 == RADAR_TRANSMIT)
    {
        adjustRadarRange();

        m_range_to_send = m_range_meters > 100000. ? 100000 : m_range_meters;
        m_range_to_send /= 1.5;
        emit signal_reqRangeChange(0,(int)m_range_to_send);
        emit signal_reqRangeChange(1,(int)m_range_to_send);
    }

    if(radar_settings.op_mode)
        ui->frameLeft->setRangeRings(10.);
    else
        ui->frameLeft->setRangeRings(ui->graphicsView->calculateRangeRing());
}

void MainWindow::calculateRadarScale()
{
    int cur_map_scale = distanceList.at(cur_zoom_lvl);
    double line_per_cur_scale = cur_map_scale / pow(2.0, 18-cur_zoom_lvl ) / 0.597164;
    double map_meter_per_pixel =  ((double)cur_map_scale)/line_per_cur_scale;
    double cur_radar_scale;

    if(radar_settings.op_mode)
    {
        for(int zoom_idx = 0; zoom_idx < distanceList.size(); zoom_idx++)
        {
            cur_zoom_lvl = zoom_idx;
            cur_map_scale = distanceList.at(zoom_idx);
            line_per_cur_scale = cur_map_scale / pow(2.0, 18-zoom_idx ) / 0.597164;
            map_meter_per_pixel =  ((double)cur_map_scale)/line_per_cur_scale;

            double cur_m_range_meters = (double)m_range_pixel*map_meter_per_pixel/2.;
            if(cur_m_range_meters < 100000.)
                break;
        }
        cur_zoom_lvl--;
        if(cur_zoom_lvl >= distanceList.size())
            cur_zoom_lvl = distanceList.size()-1;
        else if(cur_zoom_lvl < 0)
            cur_zoom_lvl = 0;

        cur_map_scale = distanceList.at(cur_zoom_lvl);
        line_per_cur_scale = cur_map_scale / pow(2.0, 18-cur_zoom_lvl ) / 0.597164;
        map_meter_per_pixel =  ((double)cur_map_scale)/line_per_cur_scale;
    }

    m_range_meters = (double)m_range_pixel*map_meter_per_pixel/2.;
    ui->graphicsView->setMapZoomLevel(cur_zoom_lvl);
    ui->graphicsView->setCurretRange(m_range_meters);

    m_tm->setTrackRangeMeter((int)(m_radar_range/1.5));
//    for (int i = 0; i < ANTENE_COUNT; ++i) {
//        m_ri->radarArpa[i]->range_meters = (int)(m_radar_range/1.5);
//    }

    if(radar_settings.op_mode)
        ui->frameLeft->setRangeText(100.,true);
    else
        ui->frameLeft->setRangeText(m_radar_range/1000.,fabs(m_radar_range-m_range_meters) < 10);

    cur_radar_scale = m_radar_range/m_range_meters;
    scene->setRadarScale(cur_radar_scale);
    if(radar_settings.op_mode)
        scene->setRings(ui->graphicsView->calculateRangePixel());

    //QPointF(-244,120) QPointF(-367,176) --> rasio = (0.6648,0.6818)
    //QPointF(156,-75) QPointF(238,-108) --> rasio = (0.6555,0.6944)
    //QPointF(-75,-82) QPointF(-115,-118) --> rasio = (0.6522,0.6949)
    //QPointF(85,93) QPointF(132,141) --> rasio = (0.6439,0.6595)

    qDebug()<<Q_FUNC_INFO<<"m_range_meters"<<m_range_meters;
    qDebug()<<Q_FUNC_INFO<<"m_range_pixel"<<m_range_pixel;
    qDebug()<<Q_FUNC_INFO<<"cur_zoom_lvl"<<cur_zoom_lvl;
    qDebug()<<Q_FUNC_INFO<<"cur_map_scale"<<cur_map_scale;
    qDebug()<<Q_FUNC_INFO<<"line_per_cur_scale"<<line_per_cur_scale;
    qDebug()<<Q_FUNC_INFO<<"map_meter_per_pixel"<<map_meter_per_pixel;
}

void MainWindow::trigger_opModeChange(bool checked)
{
    qDebug()<<Q_FUNC_INFO<<"checked"<<checked;
    radar_settings.op_mode = checked;
    trigger_rangeChange();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Log4Qt::Logger::rootLogger()->debug()<<Q_FUNC_INFO;

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
    config.setValue("radar/ip_data1",radar_settings.ip_data1);
    config.setValue("radar/ip_report1",radar_settings.ip_report1);
    config.setValue("radar/ip_command1",radar_settings.ip_command1);
    config.setValue("radar/port_command1",radar_settings.port_command1);
    config.setValue("radar/port_report1",radar_settings.port_report1);
    config.setValue("radar/port_data1",radar_settings.port_data1);

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
    config.setValue("nav_sensor/hdg_auto",hdg_auto);
    config.setValue("nav_sensor/gps_auto",gps_auto);

    config.setValue("mti/enable",mti_settings.enable);
    config.setValue("mti/threshold",mti_settings.threshold);

    QStringList friendListCodeString = friendListCode.toList();
    config.setValue("friend_list",friendListCodeString);

    event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    Log4Qt::Logger::rootLogger()->trace()<<Q_FUNC_INFO<<ui->graphicsView->size().width()<<ui->graphicsView->size().height();
    m_range_pixel = qMin(ui->graphicsView->width(),ui->graphicsView->height());
//    m_range_pixel = qMax(ui->graphicsView->width(),ui->graphicsView->height());
    calculateRadarScale();
    if(radar_settings.op_mode)
        ui->frameLeft->setRangeRings(10.);
    else
        ui->frameLeft->setRangeRings(ui->graphicsView->calculateRangeRing());
}

void MainWindow::trigger_logEvent(QString msg)
{
    int nxt_idx_space = msg.indexOf(" ",20);
    QString type_section = msg.mid(20,nxt_idx_space-20);

    if(type_section == "DEBUG" ||
            type_section == "TRACE" ||
            msg.contains("QObject: Cannot create children for a parent") ||
            msg.contains("alled when already looking up or connecting/conn"))
        return;

    emit signal_trueLog(msg);

}

MainWindow::~MainWindow()
{
    delete ui;
}
