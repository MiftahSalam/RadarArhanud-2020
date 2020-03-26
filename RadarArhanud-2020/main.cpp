#include "mainwindow.h"
#include "echo/radar_global.h"

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QSettings>

QString loadStylesheetFile( const QString &path );

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString appStyle = loadStylesheetFile( ":/css/HMI_Syle.css" );
    a.setStyleSheet( appStyle );

    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");
    QFile file(config.fileName());

    if(!file.exists())
    {
        qDebug()<<"create config file";

        config.setValue("radar/show_ring",true);
        config.setValue("radar/heading_up",true);
        config.setValue("radar/show_compass",true);
        config.setValue("radar/show_heading_marker",true);
        config.setValue("radar/ip_data","127.0.0.1");
        config.setValue("radar/ip_report","127.0.0.1");
        config.setValue("radar/ip_command","236.6.7.104");
        config.setValue("radar/port_command",6136);
        config.setValue("radar/port_report",6137);
        config.setValue("radar/port_data",6135);
        config.setValue("radar/last_scale",8000);

        config.setValue("arpa/min_contour_len",3);
        config.setValue("arpa/create_arpa_by_click",true);
        config.setValue("arpa/show",true);
        config.setValue("arpa/search_radius1",10);
        config.setValue("arpa/search_radius2",20);
        config.setValue("arpa/max_target_size",50);

        config.setValue("guardZone/show",true);
        config.setValue("guardZone/enable_notif",true);
        config.setValue("guardZone/circle_type",false);
        config.setValue("guardZone/inner_range",2000);
        config.setValue("guardZone/outer_range",6700);
        config.setValue("guardZone/start_bearing",160);
        config.setValue("guardZone/end_bearing",260);
        config.setValue("guardZone/timeout",10000);
        config.setValue("guardZone/notif_thr",10000);

        config.setValue("nav_sensor/latitude",-6.9619);
        config.setValue("nav_sensor/longitude",107.7146);
        config.setValue("nav_sensor/heading",146);
        config.setValue("nav_sensor/gps_auto",true);
        config.setValue("nav_sensor/heading_auto",false);

        config.setValue("mqtt/ip","127.0.0.1");
        config.setValue("mqtt/port",18830);

        config.setValue("adsb/ip","127.0.0.1");
        config.setValue("adsb/port",18830);
        config.setValue("adsb/show",true);

        config.setValue("trail/enable",true);
        config.setValue("trail/mode",0);

        config.setValue("map/show",true);
        config.setValue("map/mode",0);

        config.setValue("mti/enable",true);
        config.setValue("mti/threshold",0);
    }
    else
    {
        radar_settings.show_rings = config.value("radar/show_ring",true).toBool();
        radar_settings.show_heading_marker = config.value("radar/show_heading_marker",true).toBool();
        radar_settings.show_compass = config.value("radar/show_compass",true).toBool();

        arpa_settings.create_arpa_by_click = config.value("arpa/create_arpa_by_click",true).toBool();
        arpa_settings.show = config.value("arpa/show",true).toBool();
        arpa_settings.min_contour_length = config.value("arpa/min_contour_len",3).toInt();
        arpa_settings.search_radius1 = config.value("arpa/search_radius1",10).toInt();
        arpa_settings.search_radius2 = config.value("arpa/search_radius2",20).toInt();
        arpa_settings.max_target_size = config.value("arpa/max_target_size",50).toInt();

        map_settings.show = config.value("map/show",true).toBool();
        map_settings.mode = (quint8)config.value("map/mode",0).toUInt();
    }

    MainWindow w;
    w.showFullScreen();

    return a.exec();
}

QString loadStylesheetFile( const QString &path )
{
    /**
     * Load application stylesheet file (.qss) from given path
     * then return the contents to function caller
     */

    QFile styleFile( path );
    if( styleFile.open( QFile::ReadOnly ) )
    {
        qDebug() << "loading stylesheet file...";
        QString style = QLatin1String( styleFile.readAll() );
        return style;
    }
    else
    {
        qDebug() << path << ": file not found!";
        return "";
    }
}
