#include "dialogselectedtrack.h"
#include "ui_dialogselectedtrack.h"

#include <QDebug>
#include <QDesktopWidget>

DialogSelectedTrack::DialogSelectedTrack(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSelectedTrack)
{
    ui->setupUi(this);
    cur_update_time = QDateTime::currentDateTime();

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerTimeout()));
}

void DialogSelectedTrack::timerTimeout()
{
    QDateTime now  = QDateTime::currentDateTime();

    qDebug()<<Q_FUNC_INFO<<now.secsTo(cur_update_time);

    if(cur_update_time.secsTo(now) > 1)
    {
        hide();
        timer->stop();
    }
    else
    {
        show();
        move(qApp->desktop()->window()->geometry().topRight()-QPoint(width(),0));
    }
}

void DialogSelectedTrack::trigger_target_select_update(
        QString id,
        QString tn,
        QString rng,
        QString brn,
        QString lat,
        QString lon,
        QString spd,
        QString crs,
        QString alt
        )
{
    cur_update_time = QDateTime::currentDateTime();

    if(!timer->isActive())
        timer->start(1000);

    ui->labelAlt->setText(alt.append(" m"));
    ui->labelBrn->setText(brn.append(176));
    ui->labelCrs->setText(crs.append(176));
    ui->labelID->setText(id);
    ui->labelLat->setText(lat);
    ui->labelLon->setText(lon);
    ui->labelRng->setText(rng.append(" Km"));
    ui->labelSpd->setText(spd.append(" kts"));
    ui->labelTn->setText(tn);
}

DialogSelectedTrack::~DialogSelectedTrack()
{
    delete ui;
}
