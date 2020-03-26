#include "frameleft.h"
#include "ui_frameleft.h"
#include "echo/radar_global.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>

FrameLeft::FrameLeft(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameLeft)
{
    ui->setupUi(this);

    ui->checkBoxShowRing->setChecked(radar_settings.show_rings);
    ui->checkBoxShowMap->setChecked(map_settings.show);
    ui->comboBoxMapMode->setCurrentIndex((int)map_settings.mode);
    ui->checkBoxShowHM->setChecked(radar_settings.show_heading_marker);
    ui->checkBoxShowCompass->setChecked(radar_settings.show_compass);
}

void FrameLeft::trigger_rangeChange(qreal range)
{
    ui->labelRingRange->setText(QString::number(range,'f',2)+" Km");
}

void FrameLeft::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug()<<Q_FUNC_INFO;

    QMenu menu;

    menu.addAction("Exit",this,SIGNAL(signal_exit()));
    menu.exec(event->pos());
}

FrameLeft::~FrameLeft()
{
    delete ui;
}

void FrameLeft::on_checkBoxShowRing_clicked(bool checked)
{
    radar_settings.show_rings = checked;
}

void FrameLeft::on_checkBoxShowMap_clicked(bool checked)
{
    map_settings.show = checked;
    emit signal_mapChange(0,checked);
}

void FrameLeft::on_comboBoxMapMode_activated(int index)
{
    map_settings.mode = (quint8)index;
    emit signal_mapChange(1,index);
}

void FrameLeft::on_checkBoxShowHM_clicked(bool checked)
{
    radar_settings.show_heading_marker = checked;
}

void FrameLeft::on_checkBoxShowCompass_clicked(bool checked)
{
    radar_settings.show_compass = checked;
}
