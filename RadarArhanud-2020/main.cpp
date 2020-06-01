#include "mainwindow.h"
#include "echo/radar_global.h"

#include <log4qt/logger.h>
#include <log4qt/propertyconfigurator.h>
#include <log4qt/loggerrepository.h>
#include <log4qt/rollingfileappender.h>

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QSettings>

QString loadStylesheetFile( const QString &path );
static void setupRootLogger(const QString &filename);
static void shutDownRootLogger();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings config(QSettings::IniFormat,QSettings::UserScope,"arhanud3_config");
    QFile file(config.fileName());

    setupRootLogger(config.fileName());

    QString appStyle = loadStylesheetFile( ":/css/HMI_Syle.css" );
    a.setStyleSheet( appStyle );

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

        config.setValue("nav_sensor/latitude",-6.9619);
        config.setValue("nav_sensor/longitude",107.7146);
        config.setValue("nav_sensor/heading",146);
        config.setValue("nav_sensor/gps_auto",true);
        config.setValue("nav_sensor/heading_auto",false);

        config.setValue("mqtt/ip","127.0.0.1");
        config.setValue("mqtt/port",18830);

        config.setValue("adsb/config","127.0.0.1;12000");
        config.setValue("adsb/type",(int)UDP);
        config.setValue("adsb/show_track",true);

        config.setValue("iff/ip","127.0.0.1");
        config.setValue("iff/port",8830);
        config.setValue("iff/show_track",true);

        config.setValue("mqtt/ip","127.0.0.1");
        config.setValue("mqtt/port",1883);
        config.setValue("mqtt/id","arhanud3");

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
        radar_settings.ip_data = config.value("radar/ip_data","127.0.0.1").toString();
        radar_settings.ip_report = config.value("radar/ip_report","127.0.0.1").toString();
        radar_settings.ip_command = config.value("radar/ip_command","236.6.7.104").toString();
        radar_settings.port_command = config.value("radar/port_command",6136).toUInt();
        radar_settings.port_report = config.value("radar/port_report",6137).toUInt();
        radar_settings.port_data = config.value("radar/port_data",6135).toUInt();

        arpa_settings.create_arpa_by_click = config.value("arpa/create_arpa_by_click",true).toBool();
        arpa_settings.show = config.value("arpa/show",true).toBool();
        arpa_settings.min_contour_length = config.value("arpa/min_contour_len",3).toInt();
        arpa_settings.search_radius1 = config.value("arpa/search_radius1",10).toInt();
        arpa_settings.search_radius2 = config.value("arpa/search_radius2",20).toInt();
        arpa_settings.max_target_size = config.value("arpa/max_target_size",50).toInt();

        trail_settings.enable = config.value("trail/enable",true).toBool();
        trail_settings.trail = config.value("trail/mode",0).toInt();

        iff_settings.ip = config.value("iff/ip","127.0.0.1").toString();
        iff_settings.port = config.value("iff/port",8330).toUInt();
        iff_settings.show_track = config.value("iff/show_track",true).toBool();

        config.setValue("mqtt/ip","127.0.0.1");
        config.setValue("mqtt/port",1883);
        config.setValue("mqtt/id","arhanud3");

        mqtt_settings.ip = config.value("mqtt/ip","127.0.0.1").toString();
        mqtt_settings.port = config.value("mqtt/port",1883).toUInt();
        mqtt_settings.id = config.value("mqtt/id",true).toString();

        adsb_settings.config = config.value("adsb/config","127.0.0.1;12000").toString();
        adsb_settings.type = (StreamType)config.value("adsb/type",QVariant(-1)).toInt();
        adsb_settings.show_track = config.value("adsb/show_track",true).toBool();

        map_settings.show = config.value("map/show",true).toBool();
        map_settings.mode = (quint8)config.value("map/mode",0).toUInt();

        currentHeading = config.value("nav_sensor/heading",0.0).toDouble();
        currentOwnShipLat = config.value("nav_sensor/latitude",0.0).toDouble();
        currentOwnShipLon = config.value("nav_sensor/longitude",0.0).toDouble();
        gps_auto = config.value("nav_sensor/gps_auto",true).toBool();
        hdg_auto = config.value("nav_sensor/heading_auto",true).toBool();
    }

    int ret;

    MainWindow w;
    w.showFullScreen();

    ret = a.exec();

    shutDownRootLogger();

    return ret;
}

void setupRootLogger(const QString &filename)
{
    QFileInfo config_info(filename);
    QString file_dir = config_info.absolutePath();
    QString configFile = file_dir + QDir::separator() + QStringLiteral("log4qt.properties");

    if (QFile::exists(configFile))
    {
        Log4Qt::PropertyConfigurator::configure(configFile);

        auto logger = Log4Qt::Logger::rootLogger();
        auto appender = logger->appender("A2");
        Log4Qt::RollingFileAppender *rolApp = static_cast<Log4Qt::RollingFileAppender*>(appender.data());

        rolApp->setFile(QDir::homePath()+QDir::separator()+".radarlog"+QDir::separator()+"radar_A3.log");
        rolApp->activateOptions();
    }
    else
    {
        qCritical()<<"Cannot find property configurator";
        exit(1);
    }

}

void shutDownRootLogger()
{
    auto logger = Log4Qt::Logger::rootLogger();

    logger->debug("################################################################");
    logger->debug("#                          STOP                                #");
    logger->debug("################################################################");

    logger->removeAllAppenders();
    logger->loggerRepository()->shutdown();
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
        qWarning() << path << ": css file not found!";
        return "";
    }
}
