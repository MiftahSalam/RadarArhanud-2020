#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

QString loadStylesheetFile( const QString &path );

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString appStyle = loadStylesheetFile( ":/css/HMI_Syle.css" );
    a.setStyle("plastique");
    a.setStyleSheet( appStyle );

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
