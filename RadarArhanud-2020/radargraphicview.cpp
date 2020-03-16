#include "radargraphicview.h"
#include "radarscene.h"

#include <QScrollBar>

#include <stdlib.h>
#include <math.h>

using namespace qmapcontrol;

//ProxySetting proxy_settings;

const QList<double> distanceList = QList<double>()<<
                                                     5000000<<
                                                     2000000<<
                                                     1000000<<
                                                     1000000<<
                                                     1000000<<
                                                     100000<<
                                                     100000<<
                                                     50000<<
                                                     50000<<
                                                     10000<<
                                                     10000<<
                                                     10000<<
                                                     1000<<
                                                     1000<<
                                                     500<<
                                                     200<<
                                                     100<<
                                                     50<<
                                                     25;

RadarGraphicView::RadarGraphicView(QWidget *parent) :
    QGraphicsView(parent)
{  
    echo = new RadarWidget(this);
    connect(echo,SIGNAL(signal_updateRadarEcho()),
            this,SLOT(update()));
    echo->hide();

    RadarScene *scene = new RadarScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(FullViewportUpdate);
    setTransformationAnchor(AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setScene(scene);

    tr1 = new ArpaTrackItem();
    scene->addItem(tr1);

    mc = new MapControl(QSize(width(),height()), MapControl::None, false, false, this);
    mc->showCrosshairs(true);
    mc->enablePersistentCache();

//    if(proxy_settings.enable)
//        mc->setProxy(proxy_settings.host,proxy_settings.port,proxy_settings.user,proxy_settings.password);

    /*
    */
    mapadapter = new GoogleMapAdapter(GoogleMapAdapter::satellite);

    l = new MapLayer("Custom Layer", mapadapter);

    mc->addLayer(l);
    mc->setView(QPointF(107.6090623,-6.88818));
    mc->setZoom(10);
    mc->enableMouseWheelEvents(false);
    mc->hide();

    connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    timer.start(1000);
}
/**/
void RadarGraphicView::drawForeground(QPainter *painter, const QRectF &rect)
{
//    qDebug()<<Q_FUNC_INFO<<rect<<scene()->sceneRect()<<echo->geometry()<<mc->loadingQueueSize();

    painter->drawPixmap(echo->geometry(),echo->grab(mc->geometry()));

    int side = (int)qMin(rect.width(),rect.height())/2;

    painter->setPen(Qt::SolidLine);
    painter->setPen(QColor(255,255,0,255));
    painter->translate((int)rect.width()/2,(int)rect.height()/2);
    centerOn(rect.width()/2,rect.height()/2);

    //compass ring text
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

        font.setPixelSize(12);
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

void RadarGraphicView::drawBackground(QPainter *painter, const QRectF &rect)
{
//    qDebug()<<Q_FUNC_INFO<<rect<<scene()->sceneRect();

    /*
    */
    if(mc->loadingQueueSize() == 0)
    {
        qDebug()<<Q_FUNC_INFO<<"map loaded"<<mc->geometry().size();
        painter->drawPixmap(mc->geometry(),mc->grab(mc->geometry()));
    }
}

void RadarGraphicView::onTimeOut()
{
    qDebug()<<Q_FUNC_INFO;
    if(mc->loadingQueueSize() == 0)
    {
        qDebug()<<Q_FUNC_INFO<<"map loaded"<<mc->geometry().size();
        invalidateScene(sceneRect(),QGraphicsScene::BackgroundLayer);
    }
}

void RadarGraphicView::resizeEvent(QResizeEvent *event)
{
    qDebug()<<Q_FUNC_INFO;

    if (scene())
    {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));

        mc->resize(event->size());
        echo->resize(event->size());

        tr1->setPos(sceneRect().width()/2,sceneRect().height()/2);
    }
}

RadarGraphicView::~RadarGraphicView()
{
}
