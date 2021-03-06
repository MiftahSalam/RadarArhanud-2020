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

    mc = new MapControl(QSize(width(),height()), MapControl::None, false, false, this);
    mc->showCrosshairs(true);
    mc->enablePersistentCache();

    if(proxy_settings.enable)
        mc->setProxy(proxy_settings.host,proxy_settings.port,proxy_settings.username,proxy_settings.password);

    mapadapter = new GoogleMapAdapter(map_settings.mode ? GoogleMapAdapter::roadmap : GoogleMapAdapter::satellite);

    l = new MapLayer("MapLayerView", mapadapter);

//    mapCenter = QPointF(108.6090623,-5.88818);
    mapCenter = QPointF(currentOwnShipLon,currentOwnShipLat);

    map_settings.loading = true;

    mc->addLayer(l);
    mc->setView(mapCenter);
    mc->setZoom(10);
    mc->enableMouseWheelEvents(false);
    mc->hide();

    qDebug()<<Q_FUNC_INFO<<mc->loadingQueueSize();
    qDebug()<<Q_FUNC_INFO<<"map image size is"<<mapImage.size()<<mc->getLayerManager()->getImage().size();

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    timer->start(1000);
}

void RadarGraphicView::setMapZoomLevel(int index)
{
    mc->setZoom(index);
    map_settings.loading = true;
}

void RadarGraphicView::onTimeOut()
{
//    qDebug()<<Q_FUNC_INFO<<mc->loadingQueueSize()<<curLoadingMapSize<<mc->getLayerManager()->getImage().size()<<size();

//    qDebug()<<Q_FUNC_INFO<<"lat diff"<<fabs(currentOwnShipLat - curLat)<<"lon diff"<<fabs(currentOwnShipLon - curLon);
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
        QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);
        QPoint displayToImage;
        QPoint pixelPos;

        for(int i=0; i<item_list.size(); i++)
        {
            item = dynamic_cast<RadarSceneItems *>(item_list.at(i));

            if(item->getRadarItemType() == RadarSceneItems::ARPA)
            {
                ArpaTrackItem *arpa_item = dynamic_cast<ArpaTrackItem *>(item);

                if(arpa_item->m_arpa_target->getStatus() < 0)
                    scene()->removeItem(arpa_item);

                qDebug()<<Q_FUNC_INFO<<arpa_item->m_arpa_target->getStatus();
                qDebug()<<Q_FUNC_INFO<<"arpa_item"<<arpa_item->m_arpa_target->m_target_id<<"selected"<<arpa_item->getItemSelected();

                displayToImage = mc->layer("MapLayerView")->mapadapter()
                        ->coordinateToDisplay(arpa_item->m_arpa_target->blobPixelPosition());

                QPointF displayToImage_f = displayToImage-map_middle;
//                qDebug()<<Q_FUNC_INFO<<"init displayToImage_f"<<displayToImage_f;
                displayToImage_f *= (3./2.);
//                qDebug()<<Q_FUNC_INFO<<"last displayToImage_f"<<displayToImage_f;

                displayToImage = displayToImage_f.toPoint()+screen_middle;
                pixelPos = displayToImage;
//                qDebug()<<Q_FUNC_INFO<<arpa_item->m_arpa_target->blobPixelPosition()<<pixelPos<<displayToImage<<screen_middle<<map_middle;
                arpa_item->setPos(pixelPos);
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
                qDebug()<<Q_FUNC_INFO<<"adsb_item"<<adsb_item<<"selected"<<adsb_item->getItemSelected();

//                qDebug()<<Q_FUNC_INFO<<time.toTime_t() - adsb_item->m_adsb_target->time_stamp;

                displayToImage = mc->layer("MapLayerView")->mapadapter()
                        ->coordinateToDisplay(QPointF(adsb_item->m_adsb_target->lon,adsb_item->m_adsb_target->lat));
                pixelPos = QPoint(displayToImage.x()+screen_middle.x()-map_middle.x(),
                                  displayToImage.y()+screen_middle.y()-map_middle.y());
                adsb_item->setPos(pixelPos);
            }
        }
        invalidateScene();
    }
}

void RadarGraphicView::trigger_mapChange(quint8 id, quint8 val)
{
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
}

int RadarGraphicView::calculateRangePixel() const
{
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

    qDebug()<<Q_FUNC_INFO<<km<<tenth_pix;

    return tenth_pix;

}
qreal RadarGraphicView::calculateRangeRing() const
{
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

    return km/5.0;
}

void RadarGraphicView::resizeEvent(QResizeEvent *event)
{
    qDebug()<<Q_FUNC_INFO<<size();

    if (scene())
    {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
    }

    mc->resize(event->size());

    QGraphicsView::resizeEvent(event);
}

void RadarGraphicView::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<Q_FUNC_INFO<<event->pos()<<mapToScene(event->pos());

    RadarSceneItems *item = dynamic_cast<RadarSceneItems *>(itemAt(event->pos()));
    bool create_arpa = true;

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
            create_arpa = false;
            tn = arpa_item->m_arpa_target->m_target_number;
            arpa_item->m_arpa_target->selected = item_selected;
        }
        else if(itemType->getRadarItemType() == RadarSceneItems::ADSB)
        {
            AdsbTrackItem *adsb_item = dynamic_cast<AdsbTrackItem *>(itemType);
            tn = adsb_item->m_adsb_target->number;
            adsb_item->m_adsb_target->selected = item_selected;
        }

        if(tn > 0)
            emit signal_selectedChange(tn,item_selected);
    }
    else
        qDebug("You didn't click on an item.");

    if(create_arpa)
    {
        qDebug("create_arpa");

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

        emit signal_reqCreateArpa(displayToCoordinat);
    }
}

void RadarGraphicView::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug()<<Q_FUNC_INFO<<event->pos()<<mapToScene(event->pos());

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
//    qDebug()<<Q_FUNC_INFO<<displayToImage<<screen_middle<<map_middle<<displayToCoordinat<<km<<bearing;
}

void RadarGraphicView::tesCreateItem()
{
    if (scene())
    {
//        scene()->addItem(new IFFTrackItem()); //temporary
//        scene()->addItem(new AdsbTrackItem()); //temporary
    }

}

RadarGraphicView::~RadarGraphicView()
{

}
