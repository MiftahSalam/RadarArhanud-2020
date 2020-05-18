#include "radargraphicview.h"
#include "echo/radar_global.h"
#include "arpatrackitem.h"
#include "ifftrackitem.h"
#include "adsbtrackitem.h"

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

//    if(proxy_settings.enable)
//        mc->setProxy(proxy_settings.host,proxy_settings.port,proxy_settings.user,proxy_settings.password);

    mapadapter = new GoogleMapAdapter(map_settings.mode ? GoogleMapAdapter::roadmap : GoogleMapAdapter::satellite);

    l = new MapLayer("MapLayerView", mapadapter);

//    mapCenter = QPointF(108.6090623,-5.88818);
    mapCenter = QPointF(107.6090623,-6.88818); //temporary
    currentOwnShipLat = mapCenter.y(); //temporary
    currentOwnShipLon = mapCenter.x(); //temporary

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

    if((mc->loadingQueueSize() != curLoadingMapSize) || map_settings.loading)
    {
        map_settings.loading = true;
        qDebug()<<Q_FUNC_INFO<<"not equal";

        if(mc->loadingQueueSize() == 0)
        {
            map_settings.loading = false;
            mapImage = mc->grab().toImage();
            curLoadingMapSize = mc->loadingQueueSize();
            emit signal_mapChange(mapImage);
            qDebug()<<Q_FUNC_INFO<<"loadingQueueSize zero and image size is"<<mapImage.size();
        }
    }

    updateSceneItems();
}

void RadarGraphicView::updateSceneItems()
{
    QList<QGraphicsItem*> item_list = items();

    if(item_list.size() > 0)
    {
        qDebug()<<Q_FUNC_INFO<<item_list.size();

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

                displayToImage = mc->layer("MapLayerView")->mapadapter()
                        ->coordinateToDisplay(arpa_item->m_arpa_target->blobPixelPosition());
                pixelPos = QPoint(displayToImage.x()+screen_middle.x()-map_middle.x(),
                                  displayToImage.y()+screen_middle.y()-map_middle.y());
//                qDebug()<<Q_FUNC_INFO<<arpa_item->m_arpa_target->blobPixelPosition()<<pixelPos;
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

                if( (time.toTime_t() - adsb_item->m_adsb_target->time_stamp) > (ADSB_TARGET_EXPIRED-5) )
                {
                    qDebug()<<Q_FUNC_INFO<<"remove adsb track"<<time.toTime_t() - adsb_item->m_adsb_target->time_stamp;
                    scene()->removeItem(adsb_item);
                }

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

qreal RadarGraphicView::calculateRangeRing() const
{
    QPoint screen_middle(width()/2,height()/2);
    QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);

    QPoint displayToImage = QPoint(-screen_middle.x()+map_middle.x(),map_middle.y());
    QPointF displayToCoordinat = mc->layer("MapLayerView")->mapadapter()->displayToCoordinate(displayToImage);

    double dif_lat = deg2rad(displayToCoordinat.y());
    double dif_lon = (deg2rad(displayToCoordinat.x()) - deg2rad(mapCenter.x()))
            * cos(deg2rad((mapCenter.y()+displayToCoordinat.y())/2.));
    double R = 6371.;

    dif_lat =  dif_lat - (deg2rad(mapCenter.y()));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
    qDebug()<<Q_FUNC_INFO<<km;

    return km/7.0;
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

    QGraphicsItem *item = itemAt(event->pos());
    bool create_arpa = true;

    if (item)
    {
        RadarSceneItems *itemType = dynamic_cast<RadarSceneItems *>(item);

        if(itemType->getRadarItemType() == RadarSceneItems::ARPA)
            create_arpa = false;
    }

    if(create_arpa)
    {
        qDebug("You didn't click on an item.");

        QPoint screen_middle(width()/2,height()/2);
        QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);

        QPoint displayToImage= QPoint(event->pos().x()-screen_middle.x()+map_middle.x(),
                                      event->pos().y()-screen_middle.y()+map_middle.y());
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
        scene()->addItem(new IFFTrackItem()); //temporary
//        scene()->addItem(new AdsbTrackItem()); //temporary
    }

}

RadarGraphicView::~RadarGraphicView()
{

}
