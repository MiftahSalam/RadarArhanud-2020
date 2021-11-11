#include "radargraphicview.h"
#include <radarengine.h>
#include "arpatrackitem.h"
#include "ifftrackitem.h"
#include "adsbtrackitem.h"

#include <log4qt/logger.h>

#include <QDebug>

RadarGraphicView::RadarGraphicView(QWidget *parent) :
    QGraphicsView(parent)
{  
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /*
    mc = new MapControl(QSize(width(),height()), MapControl::None, false, false, this);
    mc->showCrosshairs(true);
    mc->enablePersistentCache();

    if(proxy_settings.enable)
        mc->setProxy(proxy_settings.host,proxy_settings.port,proxy_settings.username,proxy_settings.password);

    mapadapter = new GoogleMapAdapter(map_settings.mode ? GoogleMapAdapter::roadmap : GoogleMapAdapter::satellite);

    l = new MapLayer("MapLayerView", mapadapter);

    mapCenter = QPointF(currentOwnShipLon,currentOwnShipLat);

    map_settings.loading = true;

    mc->addLayer(l);
    mc->setView(mapCenter);
    mc->setZoom(10);
    mc->enableMouseWheelEvents(false);
    mc->hide();

    qDebug()<<Q_FUNC_INFO<<mc->loadingQueueSize();
    qDebug()<<Q_FUNC_INFO<<"map image size is"<<mapImage.size()<<mc->getLayerManager()->getImage().size();
    */

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    timer->start(1000);
}

void RadarGraphicView::setMapZoomLevel(int index)
{
//    mc->setZoom(index);
//    map_settings.loading = true;
}

void RadarGraphicView::onTimeOut()
{
//    qDebug()<<Q_FUNC_INFO<<mc->loadingQueueSize()<<curLoadingMapSize<<mc->getLayerManager()->getImage().size()<<size();

//    qDebug()<<Q_FUNC_INFO<<"lat diff"<<fabs(currentOwnShipLat - curLat)<<"lon diff"<<fabs(currentOwnShipLon - curLon);
    /*
    if((fabs(currentOwnShipLat - mapCenter.y()) > 0.000001) || (fabs(currentOwnShipLon - mapCenter.x()) > 0.000001))
    {
        qDebug()<<Q_FUNC_INFO<<"map update view"<<currentOwnShipLat<<currentOwnShipLon;
        mapCenter.setX(currentOwnShipLon);
        mapCenter.setY(currentOwnShipLat);

        mc->setView(mapCenter);
    }

    if((mc->loadingQueueSize() != curLoadingMapSize) || map_settings.loading)
    {
        map_settings.loading = true;
        Log4Qt::Logger::rootLogger()->trace()<<Q_FUNC_INFO<<"not equal";

        if(mc->loadingQueueSize() == 0)
        {
            map_settings.loading = false;
            mapImage = mc->grab().toImage();
            curLoadingMapSize = mc->loadingQueueSize();
            emit signal_mapChange(mapImage);
            Log4Qt::Logger::rootLogger()->trace()
                    <<Q_FUNC_INFO
                   <<"loadingQueueSize zero and image size is"<<mapImage.size().width()<<mapImage.size().height();
            qDebug()<<Q_FUNC_INFO<<"map display change";
        }
    }
    */
    updateSceneItems();
}

void RadarGraphicView::showAdsb(bool show)
{
    QList<QGraphicsItem*> item_list = items();

    if(item_list.size() > 0)
    {
        RadarSceneItems *item;

        for(int i=0; i<item_list.size(); i++)
        {
            item = dynamic_cast<RadarSceneItems *>(item_list.at(i));

            if(item->getRadarItemType() == RadarSceneItems::ADSB)
            {
                AdsbTrackItem *adsb_item = dynamic_cast<AdsbTrackItem *>(item);
                adsb_item->setShow(show);
            }
        }
        invalidateScene();
    }
}

void RadarGraphicView::updateSceneItems()
{
    QList<QGraphicsItem*> item_list = items();

    if(item_list.size() > 0)
    {
//        qDebug()<<Q_FUNC_INFO<<item_list.size();

        RadarSceneItems *item;
        QPoint screen_middle(width()/2,height()/2);
//        QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);
        QPoint displayToImage;
        QPoint pixelPos;

        for(int i=0; i<item_list.size(); i++)
        {
            item = dynamic_cast<RadarSceneItems *>(item_list.at(i));

            if(item->getRadarItemType() == RadarSceneItems::ARPA)
            {
                ArpaTrackItem *arpa_item = dynamic_cast<ArpaTrackItem *>(item);

                if(arpa_item->m_cluster_track->getClusterTrackStatus() < 0)
//                    if(arpa_item->m_arpa_target->getStatus() < 0)
                    scene()->removeItem(arpa_item);
                if(arpa_item->m_arpa_target == nullptr)
                    continue;

                qDebug()<<Q_FUNC_INFO<<"arpa_item"<<arpa_item->m_arpa_target->m_target_id<<"status"<<arpa_item->m_arpa_target->getStatus();
                qDebug()<<Q_FUNC_INFO<<"arpa_item"<<arpa_item->m_arpa_target->m_target_id<<"selected"<<arpa_item->getItemSelected();

                /*
                displayToImage = mc->layer("MapLayerView")->mapadapter()
                        ->coordinateToDisplay(arpa_item->m_arpa_target->blobPixelPosition());

                QPointF displayToImage_f = displayToImage-map_middle;
                displayToImage_f *= (3./2.);

                displayToImage = displayToImage_f.toPoint()+screen_middle;
                pixelPos = displayToImage;
                arpa_item->setPos(pixelPos);
                */

//                PosPixel arpa_pos = gpsToPix(arpa_item->m_arpa_target->m_position.lat,
//                                          arpa_item->m_arpa_target->m_position.lon); //tes
                PosPixel arpa_pos = gpsToPix(arpa_item->m_arpa_target->blobPixelPosition().y(),
                                          arpa_item->m_arpa_target->blobPixelPosition().x());
                displayToImage.setX(arpa_pos.x);
                displayToImage.setY(-arpa_pos.y);

                QPointF displayToImage_f = displayToImage;
//                displayToImage_f *= (3./2.);

                displayToImage = displayToImage_f.toPoint()+screen_middle;
                pixelPos = displayToImage;
                arpa_item->setPos(pixelPos);
                qDebug()<<Q_FUNC_INFO<<arpa_item->m_arpa_target->blobPixelPosition()<<pixelPos<<displayToImage<<screen_middle;
            }
            else if(item->getRadarItemType() == RadarSceneItems::IFF)
            {
                IFFTrackItem *iff_item = dynamic_cast<IFFTrackItem *>(item);
                iff_item->setPos(sceneRect().width()/4,sceneRect().height()/4); //temporary
            }
            else if(item->getRadarItemType() == RadarSceneItems::ADSB)
            {
                QDateTime time = QDateTime::currentDateTime();
                AdsbTrackItem *adsb_item = dynamic_cast<AdsbTrackItem *>(item);

                if( (time.toTime_t() - adsb_item->m_adsb_target->time_stamp) > (ADSB_TARGET_EXPIRED-15) )
                {
                    qDebug()<<Q_FUNC_INFO<<"remove adsb track"<<time.toTime_t() - adsb_item->m_adsb_target->time_stamp;
                    scene()->removeItem(adsb_item);
                }
//                qDebug()<<Q_FUNC_INFO<<"adsb_item ptr"<<adsb_item->m_adsb_target<<"selected"<<adsb_item->getItemSelected();

//                qDebug()<<Q_FUNC_INFO<<time.toTime_t() - adsb_item->m_adsb_target->time_stamp;

                /*
                displayToImage = mc->layer("MapLayerView")->mapadapter()
                        ->coordinateToDisplay(QPointF(adsb_item->m_adsb_target->lon,adsb_item->m_adsb_target->lat));
                pixelPos = QPoint(displayToImage.x()+screen_middle.x()-map_middle.x(),
                                  displayToImage.y()+screen_middle.y()-map_middle.y());
                adsb_item->setPos(pixelPos);
                */

                PosPixel adsb_pos = gpsToPix(adsb_item->m_adsb_target->lat,adsb_item->m_adsb_target->lon);

                displayToImage.setX(adsb_pos.x);
                displayToImage.setY(-adsb_pos.y);
                displayToImage += screen_middle;
                pixelPos = displayToImage;
                adsb_item->setPos(pixelPos);
            }
        }
        invalidateScene();
    }
}

void RadarGraphicView::trigger_mapChange(quint8 id, quint8 val)
{
    /*
    if(id)
    {
        int zoom = mapadapter->adaptedZoom();
        mc->setZoom(0);

        delete mapadapter;

        if(val)
            mapadapter = new GoogleMapAdapter(GoogleMapAdapter::roadmap);
        else
            mapadapter = new GoogleMapAdapter(GoogleMapAdapter::satellite);

        l->setMapAdapter(mapadapter);
        mc->updateRequestNew();
        mc->setZoom(zoom);

        map_settings.loading = true;
    }
    */
}

int RadarGraphicView::calculateRangePixel() const
{
    /*
    QPoint screen_middle(width()/2,height()/2);
    QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);
    QPoint ref = (width() >= height()) ? QPoint(-screen_middle.x(),0) : QPoint(0,-screen_middle.y());
    QPoint displayToImage = QPoint(ref.x()+map_middle.x(),ref.y()+map_middle.y());
    QPointF displayToCoordinat = mc->layer("MapLayerView")->mapadapter()->displayToCoordinate(displayToImage);

    double dif_lat = deg2rad(displayToCoordinat.y());
    double dif_lon = (deg2rad(displayToCoordinat.x()) - deg2rad(mapCenter.x()))
            * cos(deg2rad((mapCenter.y()+displayToCoordinat.y())/2.));
    double R = 6371.;

    dif_lat =  dif_lat - (deg2rad(mapCenter.y()));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
    double ref_pix = (width() >= height()) ? fabs(ref.x()) : fabs(ref.y());
    int tenth_pix = int(10.*ref_pix/km);
    */

    const int MAX_PIX = qMin(width()/2,height()/2)-20; //test
//    const int MAX_PIX = qMin(width()/2,height()/2);
    const double KM = 10000.;
    double tenth_pix = KM*(double)MAX_PIX/currentRange;

    qDebug()<<Q_FUNC_INFO<<KM<<tenth_pix<<currentRange<<MAX_PIX;

    return (int)tenth_pix;

}
qreal RadarGraphicView::calculateRangeRing() const
{
    /*
    QPoint screen_middle(width()/2,height()/2);
    QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);
    QPoint ref = (width() <= height()) ? QPoint(-screen_middle.x(),0) : QPoint(0,-screen_middle.y());
    QPoint displayToImage = QPoint(ref.x()+map_middle.x(),ref.y()+map_middle.y());
    QPointF displayToCoordinat = mc->layer("MapLayerView")->mapadapter()->displayToCoordinate(displayToImage);

    double dif_lat = deg2rad(displayToCoordinat.y());
    double dif_lon = (deg2rad(displayToCoordinat.x()) - deg2rad(mapCenter.x()))
            * cos(deg2rad((mapCenter.y()+displayToCoordinat.y())/2.));
    double R = 6371.;

    dif_lat =  dif_lat - (deg2rad(mapCenter.y()));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
//    km /= 1.852; //NM
    qDebug()<<Q_FUNC_INFO<<km<<map_middle<<screen_middle;

    return km/10.0;
    */
}

void RadarGraphicView::resizeEvent(QResizeEvent *event)
{
    qDebug()<<Q_FUNC_INFO<<size();

    if (scene())
    {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
    }

//    mc->resize(event->size());

    QGraphicsView::resizeEvent(event);
}

void RadarGraphicView::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<Q_FUNC_INFO<<event->pos()<<mapToScene(event->pos());

    RadarSceneItems *item = dynamic_cast<RadarSceneItems *>(itemAt(event->pos()));
    bool create_arpa = false;

    if (item)
    {
        bool item_selected = !item->getItemSelected();
        item->setItemSelected(item_selected);
        qDebug()<<Q_FUNC_INFO<<"item select"<<item_selected<<item;

        foreach (QGraphicsItem *item_i, items())
        {
            RadarSceneItems *radar_item_i = dynamic_cast<RadarSceneItems *>(item_i);
            if(radar_item_i != item)
                radar_item_i->setItemSelected(false);
        }

        RadarSceneItems *itemType = dynamic_cast<RadarSceneItems *>(item);
        int tn = -1;

        if(itemType->getRadarItemType() == RadarSceneItems::ARPA)
        {
            ArpaTrackItem *arpa_item = dynamic_cast<ArpaTrackItem *>(itemType);
            if(arpa_item->m_arpa_target)
            {
                tn = arpa_item->m_arpa_target->m_target_number;
                arpa_item->m_arpa_target->selected = item_selected;
            }
        }
        else if(itemType->getRadarItemType() == RadarSceneItems::ADSB)
        {
            AdsbTrackItem *adsb_item = dynamic_cast<AdsbTrackItem *>(itemType);
            if(adsb_item->m_adsb_target)
            {
                tn = adsb_item->m_adsb_target->number;
                adsb_item->m_adsb_target->selected = item_selected;
            }
        }

        if(tn > 0)
            emit signal_selectedChange(tn,item_selected);
    }
    else
    {
        create_arpa = true;
        qDebug("You didn't click on an item.");
    }

    if(create_arpa)
    {
        qDebug("create_arpa");

        /*
        QPoint screen_middle(width()/2,height()/2);
        QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);
        QPoint event_pos_scaled(event->pos().x(),event->pos().y());
        event_pos_scaled -= screen_middle;
        QPointF event_pos_scaled_f((qreal)event_pos_scaled.x(),(qreal)event_pos_scaled.y());
        qDebug()<<Q_FUNC_INFO<<"init event_pos_scaled"<<event_pos_scaled_f;
        event_pos_scaled_f *= (2./3.);
        qDebug()<<Q_FUNC_INFO<<"last event_pos_scaled"<<event_pos_scaled_f;
        QPoint displayToImage = event_pos_scaled_f.toPoint()+map_middle;
        QPointF displayToCoordinat = mc->layer("MapLayerView")->mapadapter()->displayToCoordinate(displayToImage);
        */

        QPoint screen_middle(width()/2,height()/2);
        QPoint event_pos_scaled(event->pos().x(),event->pos().y());
        event_pos_scaled.setX(event_pos_scaled.x()-screen_middle.x());
        event_pos_scaled.setY(-event_pos_scaled.y()+screen_middle.y());

        QPointF event_pos_scaled_f((qreal)event_pos_scaled.x(),(qreal)event_pos_scaled.y());
        QPointF displayToCoordinat;

        qDebug()<<Q_FUNC_INFO<<"init event_pos_scaled"<<event_pos_scaled_f;
//        event_pos_scaled_f *= (2./3.);
        qDebug()<<Q_FUNC_INFO<<"last event_pos_scaled"<<event_pos_scaled_f;

        PosGps gps_pos = pixToGps(event_pos_scaled_f.x(),event_pos_scaled_f.y());
        displayToCoordinat.setX(gps_pos.lon);
        displayToCoordinat.setY(gps_pos.lat);

        emit signal_reqCreateArpa(displayToCoordinat);
    }
}

RadarGraphicView::PosGps RadarGraphicView::pixToGps(const int x, const int y)
{
    QPoint screen_middle(0,0);
    QPointF event_pos_scaled(x,y);
    QLineF line(screen_middle, event_pos_scaled);
    Polar pol;
    Position own_pos;
    Position pos;
    double angle = line.angle()+90.;
    const int MAX_PIX = qMin(width()/2,height()/2)-20; //tes
//    const int MAX_PIX = qMin(width()/2,height()/2);

    while (angle >=360. || angle < 0. ) {
        if(angle >= 360.)
            angle -= 360.;
        if(angle < 0.)
            angle += 360.;
    }

    own_pos.lat = currentOwnShipLat;
    own_pos.lon = currentOwnShipLon;
    pol.angle = SCALE_DEGREES_TO_RAW2048(angle);
    pol.r = static_cast<int>(line.length());
    pos.lat = own_pos.lat +
            (double)pol.r / (double)MAX_PIX * currentRange * cos(deg2rad(SCALE_RAW_TO_DEGREES2048(pol.angle))) / 60. / 1852.;
    pos.lon = own_pos.lon +
            (double)pol.r / (double)MAX_PIX * currentRange * sin(deg2rad(SCALE_RAW_TO_DEGREES2048(pol.angle))) /
            cos(deg2rad(own_pos.lat)) / 60. / 1852.;

//    qDebug()<<Q_FUNC_INFO<<"line"<<line.length()<<angle<<line;
//    qDebug()<<Q_FUNC_INFO<<"pol"<<pol.r<<pol.angle;
//    qDebug()<<Q_FUNC_INFO<<"pos"<<pos.lat<<pos.lon<<currentRange;


    PosGps pos_to_convert;
//    pos.lon = currentOwnShipLon+(static_cast<double>(x)/(currentRange*cos(deg2rad(currentOwnShipLat))));
//    pos.lat = currentOwnShipLat+(static_cast<double>(y)/currentRange);

    pos_to_convert.lat = pos.lat;
    pos_to_convert.lon = pos.lon;

    return pos_to_convert;
}

RadarGraphicView::PosPixel RadarGraphicView::gpsToPix(const double lat, const double lon)
{
    QPoint screen_middle(0,0);
    QPointF event_pos_scaled(1,1);
    QLineF line(screen_middle,event_pos_scaled);
    Polar pol;
    Position own_pos;
    Position pos;
    double angle;
    double dif_lat = lat;
    double dif_lon;
    const int MAX_PIX = qMin(width()/2,height()/2)-20; //tes
//    const int MAX_PIX = qMin(width()/2,height()/2);

    own_pos.lat = currentOwnShipLat;
    own_pos.lon = currentOwnShipLon;
    pos.lat = lat;
    pos.lon = lon;
    dif_lat -= own_pos.lat;
    dif_lon = (lon - own_pos.lon) * cos(deg2rad(own_pos.lat));
    pol.r = (int)(sqrt(dif_lat * dif_lat + dif_lon * dif_lon) * 60. * 1852. * (double)MAX_PIX / (double)currentRange + 1);
    pol.angle = (int)((atan2(dif_lon, dif_lat)) * (double)LINES_PER_ROTATION / (2. * M_PI) + 1);  // + 1 to minimize rounding errors
    if (pol.angle < 0) pol.angle += LINES_PER_ROTATION;

    angle = SCALE_RAW_TO_DEGREES2048(pol.angle)-90.;

    while (angle >=360. || angle < 0. ) {
        if(angle >= 360.)
            angle -= 360.;
        if(angle < 0.)
            angle += 360.;
    }

    line.setAngle(angle);
    line.setLength(pol.r);

    qDebug()<<Q_FUNC_INFO<<"line"<<line.length()<<angle<<line;
    qDebug()<<Q_FUNC_INFO<<"pol"<<pol.r<<pol.angle;
    qDebug()<<Q_FUNC_INFO<<"pos"<<pos.lat<<pos.lon<<currentRange;


    PosPixel pos_to_convert;
//    double dif_lat = (lat - currentOwnShipLat) * currentRange;
//    double dif_lon = (lon - currentOwnShipLon) * cos(deg2rad(currentOwnShipLat)) * currentRange;
//    pos.x = static_cast<int>(dif_lon);
//    pos.y = static_cast<int>(dif_lat);

    pos_to_convert.x = line.p2().x();
    pos_to_convert.y = line.p2().y();

    return pos_to_convert;
}

void RadarGraphicView::setCurretRange(int range)
{
    currentRange = static_cast<double>(range);
}
void RadarGraphicView::mouseMoveEvent(QMouseEvent *event)
{
    /*
    QPoint screen_middle(width()/2,height()/2);
    QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);

    QPoint displayToImage= QPoint(event->pos().x()-screen_middle.x()+map_middle.x(),
                                  event->pos().y()-screen_middle.y()+map_middle.y());
    QPointF displayToCoordinat = mc->layer("MapLayerView")->mapadapter()->displayToCoordinate(displayToImage);

    qreal longitude = displayToCoordinat.x();
    qreal latitude = displayToCoordinat.y();

    double dif_lat = deg2rad(latitude);
    double dif_lon = (deg2rad(longitude) - deg2rad(mapCenter.x()))
            * cos(deg2rad((mapCenter.y()+latitude)/2.));
    double R = 6371.;

    dif_lat =  dif_lat - (deg2rad(mapCenter.y()));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
    qreal bearing = atan2(dif_lon,dif_lat)*180./M_PI;

    while(bearing < 0.0)
    {
        bearing += 360.0;
    }

    emit signal_cursorPosition(latitude,longitude,km,bearing);
    */
//    qDebug()<<Q_FUNC_INFO<<displayToImage<<screen_middle<<map_middle<<displayToCoordinat<<km<<bearing;
//    qDebug()<<Q_FUNC_INFO<<"map control"<<latitude<<longitude<<km<<bearing;


    QPoint screen_middle(width()/2,height()/2);
    PosGps gps = pixToGps(event->pos().x()-screen_middle.x(), -event->pos().y()+screen_middle.y());
    double const R = 6371.;

    double dif_lat = deg2rad(gps.lat);
    double dif_lon = (deg2rad(gps.lon) - deg2rad(currentOwnShipLon))
            * cos(deg2rad((currentOwnShipLat+gps.lat)/2.));
    dif_lat =  dif_lat - (deg2rad(currentOwnShipLat));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
    double bearing = atan2(dif_lon,dif_lat)*180./M_PI;

    while(bearing < 0.0)
    {
        bearing += 360.0;
    }

    emit signal_cursorPosition(gps.lat,gps.lon,km,bearing);

//    qDebug()<<Q_FUNC_INFO<<"create own"<<gps.lat<<gps.lon<<km<<bearing<<currentRange;
//    qDebug()<<Q_FUNC_INFO<<"gpsToPix x"<<pix.x<<"y"<<pix.y;
//    qDebug()<<Q_FUNC_INFO<<"pixToGps lat"<<gps.lat<<"lon"<<gps.lon;
}

RadarGraphicView::~RadarGraphicView()
{

}
