#include "framebottom.h"
#include "ui_framebottom.h"
#include <radarengine.h>

#include <QMessageBox>

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

    ui->lineEditLat->setValidator(new QDoubleValidator(-90,90,6,ui->lineEditLat));
    ui->lineEditLon->setValidator(new QDoubleValidator(-180,180,6,ui->lineEditLon));
    ui->lineEditHDG->setValidator(new QDoubleValidator(0,360,1,ui->lineEditHDG));

    ui->lineEditLat->setText(QString::number(currentOwnShipLat,'f',6));
    ui->lineEditLon->setText(QString::number(currentOwnShipLon,'f',6));
    ui->lineEditHDG->setText(QString::number(currentHeading,'f',1));
    ui->lineEditGMT->setText(QDateTime::currentDateTimeUtc().time().toString("hh:mm:ss"));

    ui->checkBoxGPS->setChecked(gps_auto);
    ui->checkBoxHDG->setChecked(hdg_auto);

    m_mqtt = getMQTT();
    connect(m_mqtt,SIGNAL(messageReceived(QString)),this,SLOT(trigger_OSD_received(QString)));
    connect(m_mqtt,SIGNAL(connected()),this,SLOT(trigger_OSD_connected()));
    connect(m_mqtt,SIGNAL(connectEnable()),this,SLOT(trigger_OSD_connected()));
    connect(m_mqtt,SIGNAL(disconnected()),this,SLOT(trigger_OSD_disconnected()));
    connect(m_mqtt,SIGNAL(disconnectEnable()),this,SLOT(trigger_OSD_disconnected()));

    dataCount_mqtt_arpa = 0;
    no_hdg_count = 20;
    hdg_col_normal = true;
    no_gps_count = 20;
    gps_col_normal = true;

    if(m_mqtt->isConnected())
        qInfo()<<"Connected to nav data server";
    else
        qWarning()<<"Not connected to nav data server";

    if(hdg_auto)
    {
        ui->lineEditHDG->setReadOnly(true);
        ui->lineEditHDG->setStyleSheet("color: rgb(0,255,0);");

        if(m_mqtt->isConnected())
        {
            int ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gyro",2);
            if(ret_val != 0)
                qWarning()<<"Heading source not available";
        }
    }
    else
    {
        ui->lineEditHDG->setReadOnly(false);
        ui->lineEditHDG->setStyleSheet("color: rgb(255,255,255);");

        if(m_mqtt->isConnected())
        {
            int ret_val = m_mqtt->unsubscribe(m_mqtt->getMID(), "gyro");
            if(ret_val != 0)
                qDebug()<<"error unsubscribe from heading source";
        }
    }

    if(gps_auto)
    {
        ui->lineEditLat->setReadOnly(true);
        ui->lineEditLon->setReadOnly(true);

        if(m_mqtt->isConnected())
        {
            int ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
            if(ret_val != 0)
                qWarning()<<"GPS source not available";
        }
    }
    else
    {
       ui->lineEditLat->setReadOnly(false);
       ui->lineEditLon->setReadOnly(false);

        if(m_mqtt->isConnected())
        {
            int ret_val = m_mqtt->unsubscribe(m_mqtt->getMID(), "gps");
            if(ret_val != 0)
                qDebug()<<"error unsubscribe from GPS source";
        }

    }

    timer.start(1000);

}

void FrameBottom::trigger_OSD_received(QString msg)
{
    if(msg.contains("gyro"))
    {
        no_hdg_count = 0;
        msg.remove("gyro>");
        qDebug()<<Q_FUNC_INFO<<"hdg"<<msg;
        ui->lineEditHDG->setText(msg);
        currentHeading = msg.toDouble();
    }
    else if(msg.contains("gps>") )
    {
        no_gps_count = 0;
        qDebug()<<Q_FUNC_INFO<<"lat"<<msg;
        msg.remove("gps>");
        QStringList gpsData = msg.split("#");
        ui->lineEditLat->setText(gpsData.at(0));
        ui->lineEditLon->setText(gpsData.at(1));
        currentOwnShipLat = gpsData.at(0).toDouble();
        currentOwnShipLon = gpsData.at(1).toDouble();
    }
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


    if(m_mqtt->isConnected())
    {
        if(arpaModel->rowCount()>0 && arpaModel->rowCount()>dataCount_mqtt_arpa)
        {
            QString id,rng,brn,spd,crs,mq_data;
            QModelIndex index = arpaModel->index(dataCount_mqtt_arpa,0);
            QByteArray mq_databyte;

            id = arpaModel->data(index).toString();
            index = arpaModel->index(dataCount_mqtt_arpa,1);
            rng = arpaModel->data(index).toString();
            index = arpaModel->index(dataCount_mqtt_arpa,2);
            brn = arpaModel->data(index).toString();
            index = arpaModel->index(dataCount_mqtt_arpa,3);
            spd = arpaModel->data(index).toString();
            index = arpaModel->index(dataCount_mqtt_arpa,4);
            crs = arpaModel->data(index).toString();

            mq_data = id+"#"+rng+"#"+brn+"#"+spd+"#"+crs;
            mq_databyte = mq_data.toUtf8();
            m_mqtt->publish(m_mqtt->getMID(), "radar", mq_databyte.size(), mq_databyte.data(), 2, false);

            dataCount_mqtt_arpa++;
//            qDebug()<<"dataCount_mqtt1"<<dataCount_mqtt_arpa;
        }
    }

    if(dataCount_mqtt_arpa == arpaModel->rowCount())
    {
//        qDebug()<<"dataCount_mqtt_arpa"<<dataCount_mqtt_arpa;
        dataCount_mqtt_arpa = 0;
    }
    else if(dataCount_mqtt_arpa > arpaModel->rowCount())
    {
//        qDebug()<<"dataCount_mqtt3"<<dataCount_mqtt_arpa;
        dataCount_mqtt_arpa = arpaModel->rowCount() - 1;
        if(dataCount_mqtt_arpa<1)
            dataCount_mqtt_arpa = 0;
    }
//    qDebug()<<Q_FUNC_INFO<<target_time_tag_list.size()<<target_to_delete.size();


    ui->lineEditGMT->setText(QDateTime::currentDateTimeUtc().time().toString("hh:mm:ss"));

    if(hdg_auto)
    {
        bool hdg_col_normal_buf;
        no_hdg_count++;
        if(no_hdg_count>200)
            no_hdg_count = 11;

        if(no_hdg_count>10)
            hdg_col_normal_buf = false;
        else
            hdg_col_normal_buf = true;

        if(hdg_col_normal_buf^hdg_col_normal)
        {
            hdg_col_normal = hdg_col_normal_buf;
            if(hdg_col_normal)
            {
                ui->lineEditHDG->setStyleSheet("color: rgb(0,255,0);");
                qInfo()<<"Heading source available";
            }
            else
            {
                ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");
                qWarning()<<"Heading source not available";
            }
        }

    }

    if(gps_auto)
    {
        bool gps_col_normal_buf;
        no_gps_count++;
        if(no_gps_count>200)
            no_gps_count = 11;

        if(no_gps_count>10)
            gps_col_normal_buf = false;
        else
            gps_col_normal_buf = true;

        if(gps_col_normal_buf^gps_col_normal)
        {
            gps_col_normal = gps_col_normal_buf;
            if(gps_col_normal)
            {
                ui->lineEditLat->setStyleSheet("color: rgb(0,255,0);");
                ui->lineEditLon->setStyleSheet("color: rgb(0,255,0);");
                qInfo()<<"GPS source available";
            }
            else
            {
                ui->lineEditLat->setStyleSheet("color: rgb(255,0,0);");
                ui->lineEditLon->setStyleSheet("color: rgb(255,0,0);");
                qWarning()<<"GPS source not available";
            }
        }

    }

    if(!m_mqtt->isConnected())
    {
        qDebug()<<"server nav data not connected";

        int con_result = m_mqtt->connect_async(mqtt_settings.ip.toUtf8().constData(), mqtt_settings.port);
        m_mqtt->loop_start();

        qDebug()<<Q_FUNC_INFO<<"reconnecting to "<<m_mqtt->getMID()<<mqtt_settings.ip<<mqtt_settings.port<<con_result;

        int ret_val;
        if(hdg_auto)
        {
            ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gyro",2);
            if(ret_val != 0)
                qInfo()<<"cannot connected to heading source";
        }

        if(gps_auto)
        {
            ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
            if(ret_val != 0)
                qInfo()<<"cannot connected to GPS source";
        }
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

void FrameBottom::on_pushButtonApply_clicked()
{
    double currentHeading_buf = ui->lineEditHDG->text().toDouble();
    double currentOwnShipLat_buf = ui->lineEditLat->text().toDouble();
    double currentOwnShipLon_buf = ui->lineEditLon->text().toDouble();

    if((currentHeading_buf < 0) || (currentHeading_buf > 360))
    {
        ui->lineEditHDG->setText(QString::number(currentHeading,'f',1));
        QMessageBox::warning(this,"Nav Data Input error","Invalid heading input.\n Input range 0 to 360");
    }
    else
        currentHeading = currentHeading_buf;

    if((currentOwnShipLat_buf < -90.0) || (currentOwnShipLat_buf > 90.0))
    {
        ui->lineEditLat->setText(QString::number(currentOwnShipLat,'f',6));
        QMessageBox::warning(this,"Nav Data Input error","Invalid Latitude input.\n Input range -90 to 90");
    }
    else
        currentOwnShipLat = currentOwnShipLat_buf;

    if((currentOwnShipLon_buf < -180.0) || (currentOwnShipLon_buf > 180.0))
    {
        ui->lineEditLon->setText(QString::number(currentOwnShipLon,'f',6));
        QMessageBox::warning(this,"Nav Data Input error","Invalid Longitude input.\n Input range -180 to 180");
    }
    else
        currentOwnShipLon = currentOwnShipLon_buf;
}

void FrameBottom::on_checkBoxGPS_clicked(bool checked)
{
    gps_auto = checked;
    no_gps_count = 20;

    int ret_val;

    if(gps_auto)
    {
        ui->lineEditLat->setReadOnly(true);
        ui->lineEditLon->setReadOnly(true);
        ui->lineEditLat->setStyleSheet("color: rgb(255,0,0);");
        ui->lineEditLon->setStyleSheet("color: rgb(255,0,0);");

        if(m_mqtt->isConnected())
        {
            ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
            if(ret_val != 0)
                qWarning()<<"GPS source not available";
        }
    }
    else
    {
       ui->lineEditLat->setReadOnly(false);
       ui->lineEditLon->setReadOnly(false);
        ui->lineEditLat->setStyleSheet("color: rgb(255,255,255);");
        ui->lineEditLon->setStyleSheet("color: rgb(255,255,255);");

        if(m_mqtt->isConnected())
        {
            ret_val = m_mqtt->unsubscribe(m_mqtt->getMID(), "gps");
            if(ret_val != 0)
                qDebug()<<"error unsubscribe from GPS source";
        }
    }

    if(!m_mqtt->isConnected())
        qWarning()<<"Not connected to nav data server";
}

void FrameBottom::on_checkBoxHDG_clicked(bool checked)
{
    hdg_auto = checked;
    no_hdg_count = 20;

    int ret_val;

    if(hdg_auto)
    {
        ui->lineEditHDG->setReadOnly(true);
        ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");

        if(m_mqtt->isConnected())
        {
            ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gyro",2);
            if(ret_val != 0)
                qWarning()<<"Heading source not available";
        }
    }
    else
    {
        ui->lineEditHDG->setReadOnly(false);
        ui->lineEditHDG->setStyleSheet("color: rgb(255,255,255);");

        if(m_mqtt->isConnected())
        {
            ret_val = m_mqtt->unsubscribe(m_mqtt->getMID(), "gyro");
            if(ret_val != 0)
                qDebug()<<"error unsubscribe from heading source";
        }
    }

    if(!m_mqtt->isConnected())
        qWarning()<<"Not connected to nav data server";
}

void FrameBottom::trigger_OSD_connected()
{
    qInfo()<<"Connected to nav data server";

    int ret_val;

    no_gps_count = 20;
    no_hdg_count = 20;

    if(gps_auto)
    {
        ui->lineEditLat->setStyleSheet("color: rgb(255,0,0);");
        ui->lineEditLon->setStyleSheet("color: rgb(255,0,0);");

        ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
        if(ret_val != 0)
            qWarning()<<"GPS source not available";

    }

    if(hdg_auto)
    {
        ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");
        ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");

        ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gyro",2);
        if(ret_val != 0)
            qWarning()<<"Heading source not available";

    }
}

void FrameBottom::trigger_OSD_disconnected()
{
    no_gps_count = 20;
    no_hdg_count = 20;

    if(gps_auto)
    {
        ui->lineEditLat->setStyleSheet("color: rgb(255,0,0);");
        ui->lineEditLon->setStyleSheet("color: rgb(255,0,0);");
    }

    if(hdg_auto)
    {
        ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");
        ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");
    }

    qWarning()<<"Disconnect from nav data server";
}
