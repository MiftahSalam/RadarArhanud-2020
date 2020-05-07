#include "radargraphicview.h"
#include "radarscene.h"

#include <QScrollBar>

#include <stdlib.h>
#include <math.h>

using namespace qmapcontrol;

//ProxySetting proxy_settings;

const QList<int> distanceList = QList<int>()<<5000000<<2000000<<1000000<<1000000<<1000000<<
                                               100000<<100000<<50000<<50000<<10000<<10000<<
                                               10000<<1000<<1000<<500<<200<<100<<50<<25;

RadarGraphicView::RadarGraphicView(QWidget *parent) :
    QGraphicsView(parent)
{  
    echo = new RadarWidget(this,10);
    connect(echo,SIGNAL(signal_updateRadarEcho()),
            this,SLOT(update()));
    connect(echo,SIGNAL(signal_zoom_change(int)),
            this,SLOT(trigger_RangeChange(int)));
    connect(this,SIGNAL(signal_reqCreateArpa(QPointF)),echo,SLOT(trigger_reqCreateArpa(QPointF)));

    echo->hide();

    RadarScene *scene = new RadarScene(this,echo->arpa);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    connect(echo,SIGNAL(signal_arpaChange(bool,int)),scene,SLOT(trigger_reqNewArpa(bool,int)));

    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(MinimalViewportUpdate);
    setTransformationAnchor(AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setScene(scene);

    scene->addItem(new IFFTrackItem()); //temporary

    mc = new MapControl(QSize(width(),height()), MapControl::None, false, false, this);
    mc->showCrosshairs(true);
    mc->enablePersistentCache();

//    if(proxy_settings.enable)
//        mc->setProxy(proxy_settings.host,proxy_settings.port,proxy_settings.user,proxy_settings.password);

    /*
    */
    mapadapter = new GoogleMapAdapter(map_settings.mode ? GoogleMapAdapter::roadmap : GoogleMapAdapter::satellite);

    l = new MapLayer("MapLayerView", mapadapter);

//    mapCenter = QPointF(108.6090623,-5.88818);
    mapCenter = QPointF(107.6090623,-6.88818); //temporary
    currentOwnShipLat = mapCenter.y(); //temporary
    currentOwnShipLon = mapCenter.x(); //temporary

    mc->addLayer(l);
    mc->setView(mapCenter);
    mc->setZoom(10);
    mc->enableMouseWheelEvents(false);
//    mc->show();
    mc->hide();

    currentCursor.cursorMoveTime = QTime::currentTime().addSecs(-5);

    setMouseTracking(true);

    connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    timer.start(1000);
}

void RadarGraphicView::updateArpaItem()
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

                displayToImage = mc->layer("MapLayerView")->mapadapter()
                        ->coordinateToDisplay(QPointF(arpa_item->m_arpa_target->m_position.lon,
                                                      arpa_item->m_arpa_target->m_position.lat));
                pixelPos = QPoint(displayToImage.x()+screen_middle.x()-map_middle.x(),
                                  displayToImage.y()+screen_middle.y()-map_middle.y());

    //            qDebug()<<Q_FUNC_INFO<<arpa_item->m_arpa_target->m_target_id<<arpa_item->m_arpa_target->getStatus();
                arpa_item->setPos(pixelPos);
            }
            else if(item->getRadarItemType() == RadarSceneItems::IFF)
            {
                IFFTrackItem *iff_item = dynamic_cast<IFFTrackItem *>(item);
                iff_item->setPos(sceneRect().width()/4,sceneRect().height()/4); //temporary
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

        loadMapFinish = false;
        renderMapFinish = false;
    }
    invalidateScene(sceneRect(),QGraphicsScene::BackgroundLayer);
}

void RadarGraphicView::trigger_RangeChange(int zoom_lvl)
{
    qDebug()<<Q_FUNC_INFO<<zoom_lvl<<radar_settings.last_scale<<calculateRangeRing();

    loadMapFinish = false;
    renderMapFinish = false;
    mc->setZoom(zoom_lvl);

    emit signal_rangeChange(calculateRangeRing());
}

qreal RadarGraphicView::calculateRangeRing()
{
    QPoint screen_middle(width()/2,height()/2);
    QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);

    QPoint displayToImage= QPoint(-screen_middle.x()+map_middle.x(),map_middle.y());
    QPointF displayToCoordinat = mc->layer("MapLayerView")->mapadapter()->displayToCoordinate(displayToImage);

    double dif_lat = deg2rad(displayToCoordinat.y());
    double dif_lon = (deg2rad(displayToCoordinat.x()) - deg2rad(mapCenter.x()))
            * cos(deg2rad((mapCenter.y()+displayToCoordinat.y())/2.));
    double R = 6371.;

    dif_lat =  dif_lat - (deg2rad(mapCenter.y()));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;

    return km/7.0;
}

/**/
void RadarGraphicView::drawForeground(QPainter *painter, const QRectF &rect)
{
//    qDebug()<<Q_FUNC_INFO<<rect<<scene()->sceneRect()<<echo->geometry()<<mc->loadingQueueSize()<<geometry();

    painter->drawPixmap(echo->geometry(),echo->grab(echo->geometry()));

    int side = (int)qMin(rect.width(),rect.height())/2;
    int side_max = (int)qMax(rect.width(),rect.height());

    QFont font;
    QPen pen;

    painter->translate((int)rect.width()/2,(int)rect.height()/2);
    centerOn(rect.width()/2,rect.height()/2);

    //compass ring text
    pen.setWidth(3);
    pen.setColor(Qt::green);

    painter->setPen(pen);

    if(radar_settings.show_compass)
    {
        QString text;
        for(int j=0;j<12;j++)
        {
            if(j<9)
                QTextStream(&text)<<(j*30)+90;
            else
                QTextStream(&text)<<(j*30)-270;

            int marginY = 5;
            int marginX = 15;
            QRect rect1((side-20)*cos((j*30)*M_PI/180)-marginX,
                       ((side-20)*sin((j*30)*M_PI/180)-marginY),
                       30,
                       15);
            QTextOption opt;
            opt.setAlignment(Qt::AlignHCenter);
            QFont font;

            font.setPixelSize(15);
            painter->setFont(font);
            painter->drawText(rect1,text,opt);
            text.clear();
        }

        //compass ring
        for(int j=0;j<180;j++)
        {
            int marginBig = 10;
            int marginSmall = 5;

            if(j%15==0)
                painter->drawLine(0,side,0,side-marginBig);
            else
                painter->drawLine(0,side,0,side-marginSmall);

            painter->rotate(2);
        }
    }

    //Range rings
    if(radar_settings.show_rings)
    {
        pen = painter->pen();
        pen.setWidth(1);

        painter->setPen(pen);

        int ring_margin = qCeil(side_max/7);
        int bufRng = ring_margin;
        while(bufRng < side_max)
        {
            painter->drawEllipse(-bufRng/2,-bufRng/2,bufRng,bufRng);
            bufRng += ring_margin;
        }
    }

    QTime now = QTime::currentTime();
    if(currentCursor.cursorMoveTime.secsTo(now) < 5)
    {
//        qDebug()<<Q_FUNC_INFO<<cursorMoveTime.secsTo(now);

        pen = painter->pen();
        pen.setWidth(3);

        if(map_settings.show)
            pen.setColor(Qt::black);

        font.setPixelSize(15);
        font.setBold(true);

        painter->setPen(pen);
        painter->setFont(font);
        painter->drawText((-rect.width()/2)+10,(rect.height()/2)-50,"Cursor:");
        QString lat_lon = QString("Latitude : %1 \t Longitude: %2")
                .arg(QString::number(currentCursor.latitude,'f',6))
                .arg(QString::number(currentCursor.longitude,'f',6));
        QString rng_brn = QString("Range : %1 Km \t Bearing: %2%3")
                .arg(QString::number(currentCursor.range,'f',1))
                .arg(QString::number(currentCursor.bearing,'f',1))
                .arg(176);
        painter->drawText((-rect.width()/2)+10,(rect.height()/2)-35,lat_lon);
        painter->drawText((-rect.width()/2)+10,(rect.height()/2)-20,rng_brn);
    }

    if(map_settings.show)
    {
        if(!loadMapFinish)
        {
            pen.setColor(Qt::green);

            font.setPixelSize(20);
            font.setBold(true);

            painter->setPen(pen);
            painter->setFont(font);

            painter->drawText(-50,20,"Loading Map");
        }
    }

    if(radar_settings.show_heading_marker)
    {
        pen = painter->pen();
        pen.setWidth(3);
        pen.setColor(Qt::green);

        painter->setPen(pen);
        painter->rotate(30);
        painter->drawLine(0,0,0,-side_max);
        painter->rotate(-30);
    }
}

void RadarGraphicView::drawBackground(QPainter *painter, const QRectF &rect)
{
//    qDebug()<<Q_FUNC_INFO<<rect<<scene()->sceneRect()<<width()<<height();

    /*
    */
    if(map_settings.show)
    {
        if(loadMapFinish && !renderMapFinish)
        {
            qDebug()<<Q_FUNC_INFO<<"map loaded"<<mc->geometry()<<rect;

            QRect source_rect = QRect(0,0,mc->width(),mc->height());
            QRect target_rect = QRect(0,0,(int)scene()->sceneRect().width(),(int)scene()->sceneRect().height());
            /*
            QRect target_rect = QRect((int)-scene()->sceneRect().width(),(int)-scene()->sceneRect().height(),
                                        (int)scene()->sceneRect().width()*2,(int)scene()->sceneRect().height()*2); //for relative display mode
            */
            QPixmap map_pix = mc->grab(source_rect);

//            painter->translate((int)scene()->sceneRect().width()/2,(int)scene()->sceneRect().height()/2); //for relative display mode
//            painter->rotate(90.0); //for relative display mode
            painter->drawPixmap(target_rect,map_pix,source_rect);

            renderMapFinish = true;
        }
    }
    else
    {
        painter->fillRect(rect,Qt::black);
    }
}

void RadarGraphicView::onTimeOut()
{
    qDebug()<<Q_FUNC_INFO<<mc->loadingQueueSize();
    if((mc->loadingQueueSize() == 0) && !loadMapFinish)
    {
        loadMapFinish = true;
        qDebug()<<Q_FUNC_INFO<<"map loaded"<<mc->geometry().size();
        invalidateScene(sceneRect(),QGraphicsScene::BackgroundLayer);
    }

    updateArpaItem();
}

void RadarGraphicView::resizeEvent(QResizeEvent *event)
{
    qDebug()<<Q_FUNC_INFO;

    if (scene())
    {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));

        mc->resize(2*event->size());
        echo->resize(event->size());

    }
}

void RadarGraphicView::mouseReleaseEvent(QMouseEvent *event)
{
//    qDebug()<<Q_FUNC_INFO<<event->pos()<<mapToScene(event->pos());


    QGraphicsItem *item = itemAt(event->pos());
    if (!item)
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
    currentCursor.cursorMoveTime = QTime::currentTime();

    // click coordinate to image coordinate
    QPoint screen_middle(width()/2,height()/2);
    QPoint map_middle = mc->layer("MapLayerView")->mapadapter()->coordinateToDisplay(mapCenter);

    QPoint displayToImage= QPoint(event->pos().x()-screen_middle.x()+map_middle.x(),
                                  event->pos().y()-screen_middle.y()+map_middle.y());
    QPointF displayToCoordinat = mc->layer("MapLayerView")->mapadapter()->displayToCoordinate(displayToImage);

    currentCursor.longitude = displayToCoordinat.x();
    currentCursor.latitude = displayToCoordinat.y();

    double dif_lat = deg2rad(currentCursor.latitude);
    double dif_lon = (deg2rad(currentCursor.longitude) - deg2rad(mapCenter.x()))
            * cos(deg2rad((mapCenter.y()+currentCursor.latitude)/2.));
    double R = 6371.;

    dif_lat =  dif_lat - (deg2rad(mapCenter.y()));

    double km = sqrt(dif_lat * dif_lat + dif_lon * dif_lon)*R;
    qreal bearing = atan2(dif_lon,dif_lat)*180./M_PI;

    /*
    */
    while(bearing < 0.0)
    {
        bearing += 360.0;
    }

    currentCursor.bearing = bearing;
    currentCursor.range = km;

        //    qDebug()<<Q_FUNC_INFO<<displayToImage<<screen_middle<<map_middle<<displayToCoordinat<<km<<bearing;
}


RadarGraphicView::~RadarGraphicView()
{

}
