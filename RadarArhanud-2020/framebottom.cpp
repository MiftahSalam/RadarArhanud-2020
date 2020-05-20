#include "framebottom.h"
#include "ui_framebottom.h"
#include "echo/radar.h"

FrameBottom::FrameBottom(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameBottom)
{
    ui->setupUi(this);

    connect(&timer,SIGNAL(timeout()),this,SLOT(timeoutUpdate()));

    target_arpa_time_tag_list.clear();
    target_adsb_time_tag_list.clear();

    QStandardItem *item1 = new QStandardItem("ID");
    QStandardItem *item2 = new QStandardItem("Range (Km)");
    QStandardItem *item3 = new QStandardItem("Bearing (deg)");
    QStandardItem *item4 = new QStandardItem("Speed (kts)");
    QStandardItem *item5 = new QStandardItem("Course (deg)");

    arpaModel = new QStandardItemModel(this);
    arpaModel->setColumnCount(5);
    arpaModel->setHorizontalHeaderItem(0,item1);
    arpaModel->setHorizontalHeaderItem(1,item2);
    arpaModel->setHorizontalHeaderItem(2,item3);
    arpaModel->setHorizontalHeaderItem(3,item4);
    arpaModel->setHorizontalHeaderItem(4,item5);

    QStandardItem *item6 = new QStandardItem("ICAO");
    QStandardItem *item7 = new QStandardItem("Range (Km)");
    QStandardItem *item8 = new QStandardItem("Bearing (deg)");
    QStandardItem *item9 = new QStandardItem("Altitude (m)");
    QStandardItem *item10 = new QStandardItem("Speed (kts)");
    QStandardItem *item11 = new QStandardItem("Course (deg)");
    QStandardItem *item12 = new QStandardItem("Callsign");
    QStandardItem *item13 = new QStandardItem("Country");

    adsbModel = new QStandardItemModel(this);
    adsbModel->setColumnCount(8);
    adsbModel->setHorizontalHeaderItem(0,item6);
    adsbModel->setHorizontalHeaderItem(1,item7);
    adsbModel->setHorizontalHeaderItem(2,item8);
    adsbModel->setHorizontalHeaderItem(3,item9);
    adsbModel->setHorizontalHeaderItem(4,item10);
    adsbModel->setHorizontalHeaderItem(5,item11);
    adsbModel->setHorizontalHeaderItem(6,item12);
    adsbModel->setHorizontalHeaderItem(7,item13);

    ui->tableViewTrackArpa->setModel(arpaModel);
    ui->tableViewTrackAdsb->setModel(adsbModel);

    timer.start(1000);

}

void FrameBottom::trigger_adsb_target_update(
        quint32 icao,
        double rng,
        double brn,
        double lat,
        double lon,
        double spd,
        double crs,
        double alt,
        QString call_sign,
        QString country)
{
    /*
    qDebug()<<"trigger_adsb_target_update"<<"curTarget icao"<<QString::number((qint32)icao,16);
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->CallSign str"<<call_sign;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->Lat"<<lat;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->Lon"<<lon;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->Rng"<<rng;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->bearing"<<brn;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->alt"<<alt;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->COG"<<crs;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->SOG"<<spd;
    qDebug()<<"trigger_adsb_target_update"<<"curTarget->contry str"<<country;
    */

    if(icao == UINT32_MAX)
        return;

    quint64 new_target_tt;
    if(target_adsb_time_tag_list.contains(icao))
    {
        new_target_tt = QDateTime::currentSecsSinceEpoch();
        target_adsb_time_tag_list.remove(icao);
        target_adsb_time_tag_list.insert(icao,new_target_tt);

        QList<QStandardItem *> listTarget = adsbModel->findItems(QString::number(icao,16),0);
        if(!listTarget.isEmpty())
        {
            const float MAX_FLOAT = std::numeric_limits<float>::max();
            int row = listTarget.at(0)->row();

            adsbModel->setData(arpaModel->index(row,0,QModelIndex()),
                           QString::number(icao,16));

            if(rng == NAN || rng == INFINITY)
            {
                adsbModel->setData(adsbModel->index(row,1,QModelIndex()),"-");
                adsbModel->setData(adsbModel->index(row,2,QModelIndex()),"-");
            }
            else
            {
                adsbModel->setData(adsbModel->index(row,1,QModelIndex()),
                                   QString::number(rng,'f',1));
                adsbModel->setData(adsbModel->index(row,2,QModelIndex()),
                                   QString::number(brn,'f',1));
            }

            if(alt == NAN || alt == INFINITY || alt == MAX_FLOAT)
                adsbModel->setData(adsbModel->index(row,3,QModelIndex()),"-");
            else
                adsbModel->setData(adsbModel->index(row,3,QModelIndex()),
                                   QString::number(alt,'f',1));

            if(spd == NAN || spd == INFINITY || spd == MAX_FLOAT)
            {
                adsbModel->setData(adsbModel->index(row,4,QModelIndex()),"-");
                adsbModel->setData(adsbModel->index(row,5,QModelIndex()),"-");
            }
            else
            {
                adsbModel->setData(adsbModel->index(row,4,QModelIndex()),
                                   QString::number(spd,'f',1));
                adsbModel->setData(adsbModel->index(row,5,QModelIndex()),
                                   QString::number(crs,'f',1));
            }

            adsbModel->setData(adsbModel->index(row,6,QModelIndex()),call_sign);
            adsbModel->setData(adsbModel->index(row,7,QModelIndex()),country);
        }
        else
            insertADSBList(icao,rng,brn,lat,lon,spd,crs,alt,call_sign,country);
    }
    else
    {
        target_adsb_time_tag_list.insert(icao,QDateTime::currentSecsSinceEpoch());
        insertADSBList(icao,rng,brn,lat,lon,spd,crs,alt,call_sign,country);
    }
}

void FrameBottom::insertADSBList(
        quint32 icao,
        double rng,
        double brn,
        double lat,
        double lon,
        double spd,
        double crs,
        double alt,
        QString call_sign,
        QString country)
{
    /*
    */
    adsbModel->insertRow(adsbModel->rowCount(),QModelIndex());

    const float MAX_FLOAT = std::numeric_limits<float>::max();
    int row = adsbModel->rowCount()-1;

    adsbModel->setData(adsbModel->index(row,0,QModelIndex()),
                   QString::number(icao,16));

    if(rng == NAN || rng == INFINITY)
    {
        adsbModel->setData(adsbModel->index(row,1,QModelIndex()),"-");
        adsbModel->setData(adsbModel->index(row,2,QModelIndex()),"-");
    }
    else
    {
        adsbModel->setData(adsbModel->index(row,1,QModelIndex()),
                           QString::number(rng,'f',1));
        adsbModel->setData(adsbModel->index(row,2,QModelIndex()),
                           QString::number(brn,'f',1));
    }

    if(alt == NAN || alt == INFINITY || alt == MAX_FLOAT)
        adsbModel->setData(adsbModel->index(row,3,QModelIndex()),"-");
    else
        adsbModel->setData(adsbModel->index(row,3,QModelIndex()),
                           QString::number(alt,'f',1));

    if(spd == NAN || spd == INFINITY || spd == MAX_FLOAT)
    {
        adsbModel->setData(adsbModel->index(row,4,QModelIndex()),"-");
        adsbModel->setData(adsbModel->index(row,5,QModelIndex()),"-");
    }
    else
    {
        adsbModel->setData(adsbModel->index(row,4,QModelIndex()),
                           QString::number(spd,'f',1));
        adsbModel->setData(adsbModel->index(row,5,QModelIndex()),
                           QString::number(crs,'f',1));
    }

    adsbModel->setData(adsbModel->index(row,6,QModelIndex()),call_sign);
    adsbModel->setData(adsbModel->index(row,7,QModelIndex()),country);

    adsbModel->item(row,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    adsbModel->item(row,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    adsbModel->item(row,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    adsbModel->item(row,3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    adsbModel->item(row,4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    adsbModel->item(row,5)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    adsbModel->item(row,6)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    adsbModel->item(row,7)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

}

void FrameBottom::trigger_arpa_target_param(int id, double rng, double brn, double lat, double lon, double spd, double crs)
{
//    qDebug()<<Q_FUNC_INFO<<target_arpa_time_tag_list;
//    qDebug()<<Q_FUNC_INFO<<id<<rng<<brn<<lat<<lon<<spd<<crs;

    quint64 new_target_tt;
    if(target_arpa_time_tag_list.contains(id))
    {
        new_target_tt = QDateTime::currentSecsSinceEpoch();
        target_arpa_time_tag_list.remove(id);
        target_arpa_time_tag_list.insert(id,new_target_tt);

        QList<QStandardItem *> listTarget = arpaModel->findItems(QString::number(id),0);
        if(!listTarget.isEmpty())
        {

            int row = listTarget.at(0)->row();

            arpaModel->setData(arpaModel->index(row,0,QModelIndex()),
                           id);
            arpaModel->setData(arpaModel->index(row,1,QModelIndex()),
                           QString::number(rng,'f',1));
            arpaModel->setData(arpaModel->index(row,2,QModelIndex()),
                           QString::number(brn,'f',1));
            arpaModel->setData(arpaModel->index(row,3,QModelIndex()),
                           QString::number(spd,'f',1));
            arpaModel->setData(arpaModel->index(row,4,QModelIndex()),
                           QString::number(crs,'f',1));
        }
        else
            insertArpaList(id, rng, brn, lat, lon, spd, crs);
    }
    else
    {
        target_arpa_time_tag_list.insert(id,QDateTime::currentSecsSinceEpoch());
        insertArpaList(id, rng, brn, lat, lon, spd, crs);
    }


}

void FrameBottom::insertArpaList(int id, double rng, double brn, double lat, double lon, double spd, double crs)
{
    /*
    */
    arpaModel->insertRow(arpaModel->rowCount(),QModelIndex());
    arpaModel->setData(arpaModel->index(arpaModel->rowCount()-1,0,QModelIndex()),
                   id);
    arpaModel->setData(arpaModel->index(arpaModel->rowCount()-1,1,QModelIndex()),
                   QString::number(rng,'f',1));
    arpaModel->setData(arpaModel->index(arpaModel->rowCount()-1,2,QModelIndex()),
                   QString::number(brn,'f',1));
    arpaModel->setData(arpaModel->index(arpaModel->rowCount()-1,3,QModelIndex()),
                   QString::number(spd,'f',1));
    arpaModel->setData(arpaModel->index(arpaModel->rowCount()-1,4,QModelIndex()),
                   QString::number(crs,'f',1));

    arpaModel->item(arpaModel->rowCount()-1,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    arpaModel->item(arpaModel->rowCount()-1,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    arpaModel->item(arpaModel->rowCount()-1,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    arpaModel->item(arpaModel->rowCount()-1,3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    arpaModel->item(arpaModel->rowCount()-1,4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

void FrameBottom::timeoutUpdate()
{
//    qDebug()<<Q_FUNC_INFO;

    QHashIterator<int,quint64> i(target_arpa_time_tag_list);
    QList<int> target_to_delete;
    quint64 now = QDateTime::currentSecsSinceEpoch();

    target_to_delete.clear();
    while(i.hasNext())
    {
        i.next();
        if(now-i.value()>1)
            target_to_delete.append(i.key());
    }

    for(int i=0;i<target_to_delete.size();i++)
    {
        target_arpa_time_tag_list.remove(target_to_delete.at(i));

        QList<QStandardItem *> listTarget = arpaModel->findItems(QString::number(target_to_delete.at(i)),0);
        if(!listTarget.isEmpty())
        {
            int row = listTarget.at(0)->row();
            arpaModel->removeRow(row);
        }
    }

    QHashIterator<quint32,quint64> i_adsb(target_adsb_time_tag_list);
    QList<quint32> target_adsb_to_delete;

    target_adsb_to_delete.clear();
    while(i_adsb.hasNext())
    {
        i_adsb.next();
        if(now-i_adsb.value()>60)
            target_adsb_to_delete.append(i_adsb.key());
    }

    for(int i=0;i<target_adsb_to_delete.size();i++)
    {
        quint32 cur_icao= target_adsb_to_delete.at(i);
        target_adsb_time_tag_list.remove(cur_icao);

        QList<QStandardItem *> listTarget = adsbModel->findItems(QString::number(cur_icao,16),0);
        if(!listTarget.isEmpty())
        {
            int row = listTarget.at(0)->row();
            adsbModel->removeRow(row);
        }
        emit signal_request_del_adsb_track(cur_icao);
    }

}

FrameBottom::~FrameBottom()
{
    delete ui;
}

void FrameBottom::on_pushButtonDelSel_clicked()
{
    int row_count = arpaModel->rowCount();
    if(row_count>0)
    {
        int row = ui->tableViewTrackArpa->currentIndex().row();
        int id = arpaModel->index(row,0).data().toInt();

        arpaModel->removeRow(row);
        target_arpa_time_tag_list.remove(id);

        emit signal_request_del_track(id);
    }
}

void FrameBottom::on_pushButtonDelAll_clicked()
{
    int row_count = arpaModel->rowCount();
    if(row_count>0)
    {
        target_arpa_time_tag_list.clear();
        arpaModel->removeRows(0,row_count);

        emit signal_request_del_track(-100);
    }

}
