#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->gridLayout->removeWidget(ui->graphicsView);
    ui->gridLayout->removeWidget(ui->frameBottom);
    ui->gridLayout->removeWidget(ui->frameLeft);

    ui->gridLayout->addWidget(ui->graphicsView,0,1,4,10);
    ui->gridLayout->addWidget(ui->frameBottom,4,1,1,10);
    ui->gridLayout->addWidget(ui->frameLeft,0,0,5,1);
    /*
    ui->gridLayout->addWidget(ui->graphicsView,0,0,3,3);
    ui->gridLayout->addWidget(ui->frame,3,0,1,3);
    ui->gridLayout->addWidget(ui->frame_2,0,3,4,1);
    */

    connect(ui->frameLeft,SIGNAL(signal_exit()),this,SLOT(close()));
    connect(ui->frameLeft,SIGNAL(signal_mapChange(quint8,quint8)),
            ui->graphicsView,SLOT(trigger_mapChange(quint8,quint8)));
    connect(ui->graphicsView,SIGNAL(signal_rangeChange(qreal)),ui->frameLeft,SLOT(trigger_rangeChange(qreal)));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<Q_FUNC_INFO;

    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");

    config.setValue("radar/show_ring",radar_settings.show_rings);
    config.setValue("radar/show_heading_marker",radar_settings.show_heading_marker);
    config.setValue("radar/show_compass",radar_settings.show_compass);

    config.setValue("arpa/create_arpa_by_click",arpa_settings.create_arpa_by_click);
    config.setValue("arpa/show",arpa_settings.show);
    config.setValue("arpa/min_contour_len",arpa_settings.min_contour_length);
    config.setValue("arpa/search_radius1",arpa_settings.search_radius1);
    config.setValue("arpa/search_radius2",arpa_settings.search_radius2);
    config.setValue("arpa/max_target_size",arpa_settings.max_target_size);

    config.setValue("map/show",map_settings.show);
    config.setValue("map/mode",map_settings.mode);

    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}
