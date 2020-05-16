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

    ui->tableViewTrackArpa->setModel(arpaModel);

    timer.start(1000);

}

void FrameBottom::trigger_arpa_target_param(int id, double rng, double brn, double lat, double lon, double spd, double crs)
{
//    qDebug()<<Q_FUNC_INFO<<target_arpa_time_tag_list;
//    qDebug()<<Q_FUNC_INFO<<id<<rng<<brn<<lat<<lon<<spd<<crs;

    quint64 new_target_tt;
    if(target_arpa_time_tag_list.contains(id))
    {
        new_target_tt = target_arpa_time_tag_list.value(id);
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
        if(now-i.value()>10)
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
