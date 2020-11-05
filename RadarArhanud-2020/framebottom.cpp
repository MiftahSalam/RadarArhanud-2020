#include "framebottom.h"
#include "ui_framebottom.h"
#include <radarengine.h>

#include <QMessageBox>
#include <QDesktopWidget>

FrameBottom::FrameBottom(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameBottom)
{
    ui->setupUi(this);

    if(qApp->desktop()->height() < 1000)
    {
        setMaximumHeight(280);
    }

    ui->groupBoxSubRoomStatus->hide();

    connect(&timer,SIGNAL(timeout()),this,SLOT(timeoutUpdate()));

    target_time_tag_list.clear();

    QStandardItem *item1 = new QStandardItem("ID");
    QStandardItem *item2 = new QStandardItem("Number");
    QStandardItem *item3 = new QStandardItem("Range (Km)");
    QStandardItem *item4 = new QStandardItem("Bearing (deg)");
    QStandardItem *item5 = new QStandardItem("Latitude (deg)");
    QStandardItem *item6 = new QStandardItem("Longitude (deg)");
    QStandardItem *item7 = new QStandardItem("Altitude (m)");
    QStandardItem *item8 = new QStandardItem("Speed (kts)");
    QStandardItem *item9 = new QStandardItem("Course (deg)");

    trackModel = new QStandardItemModel(this);
    trackModel->setColumnCount(7);
    trackModel->setHorizontalHeaderItem(0,item1);
    trackModel->setHorizontalHeaderItem(1,item2);
    trackModel->setHorizontalHeaderItem(2,item3);
    trackModel->setHorizontalHeaderItem(3,item4);
    trackModel->setHorizontalHeaderItem(4,item5);
    trackModel->setHorizontalHeaderItem(5,item6);
    trackModel->setHorizontalHeaderItem(6,item7);
    trackModel->setHorizontalHeaderItem(7,item8);
    trackModel->setHorizontalHeaderItem(8,item9);

    ui->tableViewTrack->setModel(trackModel);

    ui->lineEditLat->setValidator(new QDoubleValidator(-90,90,6,ui->lineEditLat));
    ui->lineEditLon->setValidator(new QDoubleValidator(-180,180,6,ui->lineEditLon));
    ui->lineEditHDG->setValidator(new QDoubleValidator(0,360,1,ui->lineEditHDG));

    ui->lineEditLat->setText(QString::number(currentOwnShipLat,'f',6));
    ui->lineEditLon->setText(QString::number(currentOwnShipLon,'f',6));
    ui->lineEditHDG->setText(QString::number(currentHeading,'f',1));
    ui->lineEditGMT->setText(QDateTime::currentDateTimeUtc().time().toString("hh:mm:ss"));

    qDebug()<<"hdg_auto"<<hdg_auto<<"gps_auto"<<gps_auto;
    ui->checkBoxGPS->setChecked(gps_auto);
    ui->checkBoxHDG->setChecked(hdg_auto);

    m_mqtt = getMQTT();
    connect(m_mqtt,SIGNAL(messageReceived(QString)),this,SLOT(trigger_OSD_received(QString)));
    connect(m_mqtt,SIGNAL(connected()),this,SLOT(trigger_OSD_connected()));
    connect(m_mqtt,SIGNAL(connectEnable()),this,SLOT(trigger_OSD_connected()));
    connect(m_mqtt,SIGNAL(disconnected()),this,SLOT(trigger_OSD_disconnected()));
    connect(m_mqtt,SIGNAL(disconnectEnable()),this,SLOT(trigger_OSD_disconnected()));

    dataCount_mqtt_track = 0;
    no_osd_count = 40;
    osd_col_normal = true;
    cur_selected_track = "-1";

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
            int ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
            if(ret_val != 0)
                qWarning()<<"Heading source not available";
        }
    }
    else
    {
        ui->lineEditHDG->setReadOnly(false);
        ui->lineEditHDG->setStyleSheet("color: rgb(255,255,255);");

        qDebug()<<"unsubscribe from nav source";
        if(m_mqtt->isConnected())
        {
            int ret_val = m_mqtt->unsubscribe(m_mqtt->getMID(), "gps");
            if(ret_val != 0)
                qDebug()<<"error unsubscribe from heading source";
        }
    }

    if(gps_auto)
    {
        ui->lineEditLat->setReadOnly(true);
        ui->lineEditLon->setReadOnly(true);
        ui->lineEditLat->setStyleSheet("color: rgb(0,255,0);");
        ui->lineEditLon->setStyleSheet("color: rgb(0,255,0);");

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
       ui->lineEditLat->setStyleSheet("color: rgb(255,255,255);");
       ui->lineEditLon->setStyleSheet("color: rgb(255,255,255);");

       qDebug()<<"unsubscribe from nav source";
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
    if(msg.contains("gps>") )
    {
        no_osd_count = 0;

        qDebug()<<Q_FUNC_INFO<<"osd"<<msg.remove("gps>");
        append_data_osd.append(msg);

        int index_hdr = append_data_osd.indexOf("?");
        if(index_hdr >= 0)
        {
            int index_end = append_data_osd.indexOf("!");
            if(index_end >= 0)
            {
                if(index_end > index_hdr)
                {
                    //?-6.939176#107.632770#31
                    append_data_osd = append_data_osd.mid(index_hdr,index_end-index_hdr);
                    qDebug()<<Q_FUNC_INFO<<"filter"<<append_data_osd.remove("!").remove("?").remove("\r").remove("\n");
                    QStringList msg_list = append_data_osd.split("#",QString::SkipEmptyParts);

                    if(msg_list.size() == 3)
                    {
                        currentOwnShipLat = msg_list.at(0).toDouble();
                        currentOwnShipLon = msg_list.at(1).toDouble();
                        currentHeading = msg_list.at(2).toDouble();

                        ui->lineEditLat->setText(msg_list.at(0));
                        ui->lineEditLon->setText(msg_list.at(1));
                        ui->lineEditHDG->setText(msg_list.at(2));
                    }
                    else
                        qDebug()<<Q_FUNC_INFO<<"osd invalid";

                    append_data_osd.clear();
                }
                else
                {
                    append_data_osd.remove(0,index_hdr);
                }
            }
//            qDebug()<<Q_FUNC_INFO<<index_end;
        }
//        qDebug()<<Q_FUNC_INFO<<index_hdr;
    }
}

void FrameBottom::trigger_target_selected(int tn, bool selected)
{
    QString str_tn = QString::number(tn);
    QList<QStandardItem *> listTarget;

    if(cur_selected_track != str_tn)
    {
        listTarget = trackModel->findItems(cur_selected_track,Qt::MatchExactly,1);

        if(listTarget.size() > 0)
        {
            int row = listTarget.at(0)->row();

            if(row >= 0)
            {
                trackModel->item(row,0)->setBackground(Qt::NoBrush);
                trackModel->item(row,1)->setBackground(Qt::NoBrush);
                trackModel->item(row,2)->setBackground(Qt::NoBrush);
                trackModel->item(row,3)->setBackground(Qt::NoBrush);
                trackModel->item(row,4)->setBackground(Qt::NoBrush);
                trackModel->item(row,5)->setBackground(Qt::NoBrush);
                trackModel->item(row,6)->setBackground(Qt::NoBrush);
                trackModel->item(row,7)->setBackground(Qt::NoBrush);
                trackModel->item(row,8)->setBackground(Qt::NoBrush);
            }
        }
    }


    listTarget = trackModel->findItems(str_tn,Qt::MatchExactly,1);

    if(listTarget.size() > 0)
    {
        int row = listTarget.at(0)->row();

        if(row >= 0)
        {
            if(selected)
            {
                cur_selected_track = str_tn;

                trackModel->item(row,0)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,1)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,2)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,3)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,4)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,5)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,6)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,7)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
                trackModel->item(row,8)->setBackground(QBrush(Qt::red,Qt::SolidPattern));
            }
            else
            {
                cur_selected_track = "-1";

                trackModel->item(row,0)->setBackground(Qt::NoBrush);
                trackModel->item(row,1)->setBackground(Qt::NoBrush);
                trackModel->item(row,2)->setBackground(Qt::NoBrush);
                trackModel->item(row,3)->setBackground(Qt::NoBrush);
                trackModel->item(row,4)->setBackground(Qt::NoBrush);
                trackModel->item(row,5)->setBackground(Qt::NoBrush);
                trackModel->item(row,6)->setBackground(Qt::NoBrush);
                trackModel->item(row,7)->setBackground(Qt::NoBrush);
                trackModel->item(row,8)->setBackground(Qt::NoBrush);
            }
        }
    }
}

void FrameBottom::trigger_target_update(
        quint32 icao,
        double rng,
        double brn,
        double lat,
        double lon,
        double spd,
        double crs,
        double alt,
        QString call_sign,
        QString country,
        bool selected)
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
//    qDebug()<<Q_FUNC_INFO<<"target_time_tag_list"<<target_time_tag_list;
    if(target_time_tag_list.contains(icao))
    {
        new_target_tt = QDateTime::currentSecsSinceEpoch();
        target_time_tag_list.remove(icao);
        target_time_tag_list.insert(icao,new_target_tt);

        QString id_track = QString::number(icao);
        QList<QStandardItem *> listTarget = trackModel->findItems(id_track,0);

        if(listTarget.isEmpty())
        {
            id_track = QString::number(icao,16).toUpper();
            listTarget = trackModel->findItems(id_track,0);
        }

        const float MAX_FLOAT = std::numeric_limits<float>::max();
        int row = listTarget.at(0)->row();

        trackModel->setData(trackModel->index(row,0,QModelIndex()),id_track);

        if(rng == NAN || rng == INFINITY)
        {
            trackModel->setData(trackModel->index(row,2,QModelIndex()),"-");
            trackModel->setData(trackModel->index(row,3,QModelIndex()),"-");
        }
        else
        {
            trackModel->setData(trackModel->index(row,2,QModelIndex()),
                                QString::number(rng,'f',1));
            trackModel->setData(trackModel->index(row,3,QModelIndex()),
                                QString::number(brn,'f',1));
        }

        if(lat == NAN || lat == INFINITY || fabs(lat) > 90.)
        {
            trackModel->setData(trackModel->index(row,4,QModelIndex()),"-");
            trackModel->setData(trackModel->index(row,5,QModelIndex()),"-");
        }
        else
        {
            trackModel->setData(trackModel->index(row,4,QModelIndex()),
                                QString::number(lat,'f',6));
            trackModel->setData(trackModel->index(row,5,QModelIndex()),
                                QString::number(lon,'f',6));
        }

        if(alt == NAN || alt == INFINITY || alt == MAX_FLOAT)
            trackModel->setData(trackModel->index(row,6,QModelIndex()),"-");
        else
            trackModel->setData(trackModel->index(row,6,QModelIndex()),
                                QString::number(alt,'f',1));

        if(spd == NAN || spd == INFINITY || spd == MAX_FLOAT)
        {
            trackModel->setData(trackModel->index(row,7,QModelIndex()),"-");
            trackModel->setData(trackModel->index(row,8,QModelIndex()),"-");
        }
        else
        {
            trackModel->setData(trackModel->index(row,7,QModelIndex()),
                                QString::number(spd,'f',1));
            trackModel->setData(trackModel->index(row,8,QModelIndex()),
                                QString::number(crs,'f',1));
        }

        //            trackModel->setData(adsbModel->index(row,8,QModelIndex()),call_sign);
        //            trackModel->setData(adsbModel->index(row,9,QModelIndex()),country);

    }
    else
    {
        target_time_tag_list.insert(icao,QDateTime::currentSecsSinceEpoch());
        insertTrackList(icao,rng,brn,lat,lon,spd,crs,alt,call_sign,country);
    }
}

void FrameBottom::insertTrackList(
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
    trackModel->insertRow(trackModel->rowCount(),QModelIndex());
    track_counter++;

    const float MAX_FLOAT = std::numeric_limits<float>::max();
    int row = trackModel->rowCount()-1;

    if(icao > 100)
        trackModel->setData(trackModel->index(row,0,QModelIndex()),
                            QString::number(icao,16).toUpper());
    else
        trackModel->setData(trackModel->index(row,0,QModelIndex()),
                            QString::number(icao));

    trackModel->setData(trackModel->index(row,1,QModelIndex()),
                        QString::number(track_counter));

    if(rng == NAN || rng == INFINITY)
    {
        trackModel->setData(trackModel->index(row,2,QModelIndex()),"-");
        trackModel->setData(trackModel->index(row,3,QModelIndex()),"-");
    }
    else
    {
        trackModel->setData(trackModel->index(row,2,QModelIndex()),
                           QString::number(rng,'f',1));
        trackModel->setData(trackModel->index(row,3,QModelIndex()),
                           QString::number(brn,'f',1));
    }

    if(lat == NAN || lat == INFINITY || fabs(lat) > 90.)
    {
        trackModel->setData(trackModel->index(row,4,QModelIndex()),"-");
        trackModel->setData(trackModel->index(row,5,QModelIndex()),"-");
    }
    else
    {
        trackModel->setData(trackModel->index(row,4,QModelIndex()),
                           QString::number(lat,'f',5));
        trackModel->setData(trackModel->index(row,5,QModelIndex()),
                           QString::number(lon,'f',5));
    }

    if(alt == NAN || alt == INFINITY || alt == MAX_FLOAT)
        trackModel->setData(trackModel->index(row,6,QModelIndex()),"-");
    else
        trackModel->setData(trackModel->index(row,6,QModelIndex()),
                           QString::number(alt,'f',1));

    if(spd == NAN || spd == INFINITY || spd == MAX_FLOAT)
    {
        trackModel->setData(trackModel->index(row,7,QModelIndex()),"-");
        trackModel->setData(trackModel->index(row,8,QModelIndex()),"-");
    }
    else
    {
        trackModel->setData(trackModel->index(row,7,QModelIndex()),
                           QString::number(spd,'f',1));
        trackModel->setData(trackModel->index(row,8,QModelIndex()),
                           QString::number(crs,'f',1));
    }

//    trackModel->setData(trackModel->index(row,8,QModelIndex()),call_sign);
//    trackModel->setData(trackModel->index(row,9,QModelIndex()),country);

    trackModel->item(row,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,4)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,5)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,6)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,7)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    trackModel->item(row,8)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
//    trackModel->item(row,8)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
//    trackModel->item(row,9)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    emit signal_update_track_num(icao,track_counter);
}

void FrameBottom::timeoutUpdate()
{
    QHashIterator<quint32,quint64> i(target_time_tag_list);
    QList<quint32> target_to_delete;
    quint64 now = QDateTime::currentSecsSinceEpoch();
    quint64 delta;

//    state_radar = RADAR_TRANSMIT; //tes
    if(state_radar != RADAR_TRANSMIT)
    {
        target_time_tag_list.clear();
        dataCount_mqtt_track = 0;

        if(trackModel->rowCount()>0)
            trackModel->removeRows(0,trackModel->rowCount());
    }

    target_to_delete.clear();
    while(i.hasNext())
    {
        i.next();
        if(i.key() > 100)
            delta = 10;
        else
            delta = 1;

        if(now-i.value()>delta)
            target_to_delete.append(i.key());
    }

    QString id_track;
    for(int i=0;i<target_to_delete.size();i++)
    {
        target_time_tag_list.remove(target_to_delete.at(i));
        id_track = QString::number(target_to_delete.at(i));

        QList<QStandardItem *> listTarget = trackModel->findItems(id_track,0);
        int row;

        if(listTarget.isEmpty())
        {
            id_track = QString::number(target_to_delete.at(i),16).toUpper();
            listTarget = trackModel->findItems(id_track,0);
            if(!listTarget.isEmpty())
                row = listTarget.at(0)->row();
        }
        else
            row = listTarget.at(0)->row();

        trackModel->removeRow(row);
        if(target_to_delete.at(i) > 100)
            emit signal_request_del_adsb_track(target_to_delete.at(i));
    }

    if(m_mqtt->isConnected())
    {
        if(trackModel->rowCount()>0 && trackModel->rowCount()>dataCount_mqtt_track)
        {
            QString id,num,rng,brn,alt,lat,lon,spd,crs,mq_data;
            QModelIndex index = trackModel->index(dataCount_mqtt_track,0);
            QByteArray mq_databyte;

            id = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,1);
            num = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,2);
            rng = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,3);
            brn = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,4);
            lat = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,5);
            lon = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,6);
            alt = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,7);
            spd = trackModel->data(index).toString();
            index = trackModel->index(dataCount_mqtt_track,8);
            crs = trackModel->data(index).toString();

            mq_data = id+"#"+num+"#"+rng+"#"+brn+"#"+lat+"#"+lon+"#"+alt+"#"+spd+"#"+crs;
            mq_databyte = mq_data.toUtf8();
            m_mqtt->publish(m_mqtt->getMID(), "track", mq_databyte.size(), mq_databyte.data(), 2, false);

            dataCount_mqtt_track++;
        }
    }

    if(dataCount_mqtt_track == trackModel->rowCount())
    {
        dataCount_mqtt_track = 0;
    }
    else if(dataCount_mqtt_track > trackModel->rowCount())
    {
        dataCount_mqtt_track = trackModel->rowCount() - 1;
        if(dataCount_mqtt_track<1)
            dataCount_mqtt_track = 0;
    }

    if(cur_selected_track != "-1")
    {
        QList<QStandardItem *> listTarget = trackModel->findItems(cur_selected_track,Qt::MatchExactly,1);

        if(listTarget.size() > 0)
        {
            int row = listTarget.at(0)->row();

            if(row >= 0)
            {
                QString id,num,rng,brn,alt,lat,lon,spd,crs;
                QModelIndex index = trackModel->index(row,0);

                id = trackModel->data(index).toString();
                index = trackModel->index(row,1);
                num = trackModel->data(index).toString();
                index = trackModel->index(row,2);
                rng = trackModel->data(index).toString();
                index = trackModel->index(row,3);
                brn = trackModel->data(index).toString();
                index = trackModel->index(row,4);
                lat = trackModel->data(index).toString();
                index = trackModel->index(row,5);
                lon = trackModel->data(index).toString();
                index = trackModel->index(row,6);
                alt = trackModel->data(index).toString();
                index = trackModel->index(row,7);
                spd = trackModel->data(index).toString();
                index = trackModel->index(row,8);
                crs = trackModel->data(index).toString();

                emit signal_target_select_update(id,cur_selected_track,rng,brn,lat,lon,spd,crs,alt);
            }
        }
    }

    ui->lineEditGMT->setText(QDateTime::currentDateTime().time().toString("hh:mm:ss"));
    if(hdg_auto && gps_auto)
    {
        bool osd_col_normal_buf;
        no_osd_count++;
        if(no_osd_count>200)
            no_osd_count = 41;

        if(no_osd_count>20)
            osd_col_normal_buf = false;
        else
            osd_col_normal_buf = true;

        if(osd_col_normal_buf != osd_col_normal)
        {
            osd_col_normal = osd_col_normal_buf;
            if(osd_col_normal)
            {
                ui->lineEditHDG->setStyleSheet("color: rgb(0,255,0);");
                ui->lineEditLat->setStyleSheet("color: rgb(0,255,0);");
                ui->lineEditLon->setStyleSheet("color: rgb(0,255,0);");
                qInfo()<<"nav source available";
            }
            else
            {
                ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");
                ui->lineEditLat->setStyleSheet("color: rgb(255,0,0);");
                ui->lineEditLon->setStyleSheet("color: rgb(255,0,0);");
                qWarning()<<"nav source not available";
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
        if(hdg_auto && gps_auto)
        {
            ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
            if(ret_val != 0)
                qDebug()<<"cannot connected to nav source";
        }
    }
}

FrameBottom::~FrameBottom()
{
    delete ui;
}

void FrameBottom::on_pushButtonDelSel_clicked()
{
    int row_count = trackModel->rowCount();
    if(row_count>0)
    {
        int row = ui->tableViewTrack->currentIndex().row();
        bool ok = false;
        quint32 id = trackModel->index(row,0).data().toUInt(&ok);

        qDebug()<<Q_FUNC_INFO<<"remove id"<<id<<ok;
        if(ok)
        {
            trackModel->removeRow(row);
            target_time_tag_list.remove(id);
            emit signal_request_del_track(id);
        }
    }
}

void FrameBottom::on_pushButtonDelAll_clicked()
{
    int row_count = trackModel->rowCount();
    if(row_count>0)
    {
        target_time_tag_list.clear();
        trackModel->removeRows(0,row_count);

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
    no_osd_count = 40;

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
                qWarning()<<"nav source not available";
        }
    }
    else
    {
        ui->lineEditLat->setReadOnly(false);
        ui->lineEditLon->setReadOnly(false);
        ui->lineEditLat->setStyleSheet("color: rgb(255,255,255);");
        ui->lineEditLon->setStyleSheet("color: rgb(255,255,255);");

        qDebug()<<"unsubscribe from nav source";
        if(m_mqtt->isConnected())
        {
            ret_val = m_mqtt->unsubscribe(m_mqtt->getMID(), "gps");
            if(ret_val != 0)
                qDebug()<<"error unsubscribe from nav source";
        }
    }

    qDebug()<<Q_FUNC_INFO<<"hdg_auto"<<hdg_auto<<"gps_auto"<<gps_auto;
    if(gps_auto != hdg_auto)
    {
        ui->checkBoxHDG->setChecked(gps_auto);
        on_checkBoxHDG_clicked(gps_auto);
    }

    if(!m_mqtt->isConnected())
        qWarning()<<"Not connected to nav data server";
}

void FrameBottom::on_checkBoxHDG_clicked(bool checked)
{
    hdg_auto = checked;
    no_osd_count = 40;

    int ret_val;

    if(hdg_auto)
    {
        ui->lineEditHDG->setReadOnly(true);
        ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");

        if(m_mqtt->isConnected())
        {
            ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
            if(ret_val != 0)
                qWarning()<<"Heading source not available";
        }
    }
    else
    {
        ui->lineEditHDG->setReadOnly(false);
        ui->lineEditHDG->setStyleSheet("color: rgb(255,255,255);");

        qDebug()<<"unsubscribe from nav source";
        if(m_mqtt->isConnected())
        {
            ret_val = m_mqtt->unsubscribe(m_mqtt->getMID(), "gps");
            if(ret_val != 0)
                qDebug()<<"error unsubscribe from heading source";
        }
    }

    qDebug()<<Q_FUNC_INFO<<"hdg_auto"<<hdg_auto<<"gps_auto"<<gps_auto;
    if(hdg_auto != gps_auto)
    {
        ui->checkBoxGPS->setChecked(hdg_auto);
        on_checkBoxGPS_clicked(hdg_auto);
    }
    if(!m_mqtt->isConnected())
        qWarning()<<"Not connected to nav data server";
}

void FrameBottom::trigger_OSD_connected()
{
    qInfo()<<"Connected to nav data server";

    int ret_val;

    no_osd_count = 40;

    if(gps_auto)
    {
        ui->lineEditLat->setStyleSheet("color: rgb(0,255,0);");
        ui->lineEditLon->setStyleSheet("color: rgb(0,255,0);");

        ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
        if(ret_val != 0)
            qWarning()<<"GPS source available";

    }

    if(hdg_auto)
    {
        ui->lineEditHDG->setStyleSheet("color: rgb(0,255,0);");

        ret_val = m_mqtt->subscribe(m_mqtt->getMID(), "gps",2);
        if(ret_val != 0)
            qWarning()<<"Heading source not available";

    }
}

void FrameBottom::trigger_OSD_disconnected()
{
    no_osd_count = 40;

    if(gps_auto)
    {
        ui->lineEditLat->setStyleSheet("color: rgb(255,0,0);");
        ui->lineEditLon->setStyleSheet("color: rgb(255,0,0);");
    }

    if(hdg_auto)
    {
        ui->lineEditHDG->setStyleSheet("color: rgb(255,0,0);");
    }

    qWarning()<<"Disconnect from nav data server";
}

int FrameBottom::getNavStatus() const
{
    return mqtt->isConnected();
//    return (no_osd_count < 20) ? 0 : 1;
}
