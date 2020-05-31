#-------------------------------------------------
#
# Project created by QtCreator 2020-03-12T14:36:06
#
#-------------------------------------------------

QT       += core gui opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadarArhanud-2020
TEMPLATE = app

DEFINES += UBUNTU16

SOURCES += main.cpp\
        mainwindow.cpp \
    frameleft.cpp \
    framebottom.cpp \
    radargraphicview.cpp \
    radarscene.cpp \
    arpatrackitem.cpp \
    echo/radar.cpp \
    ifftrackitem.cpp \
    radarsceneitems.cpp \
    adsbtrackitem.cpp \
    dialogradar.cpp \
    traildialog.cpp \
    dialogiff.cpp \
    dialogadsb.cpp \
    stream/streamdevice.cpp \
    stream/stream.cpp \
    adsb/adsbstream.cpp \
    adsb/adsb.cpp \
    qtmosq.cpp \
    dialoglogging.cpp \
    dialoglogdetail.cpp \
    dialogopenloghistory.cpp

HEADERS  += mainwindow.h \
    frameleft.h \
    framebottom.h \
    radargraphicview.h \
    radarscene.h \
    arpatrackitem.h \
    echo/radar.h \
    echo/radar_global.h \
    ifftrackitem.h \
    radarsceneitems.h \
    adsbtrackitem.h \
    dialogradar.h \
    traildialog.h \
    dialogiff.h \
    dialogadsb.h \
    stream/streamdevice.h \
    stream/stream.h \
    adsb/adsbstream.h \
    adsb/adsb.h \
    qtmosq.h \
    dialoglogging.h \
    dialoglogdetail.h \
    dialogopenloghistory.h

FORMS    += mainwindow.ui \
    frameleft.ui \
    framebottom.ui \
    dialogradar.ui \
    traildialog.ui \
    dialogiff.ui \
    dialogadsb.ui \
    dialoglogging.ui \
    dialoglogdetail.ui \
    dialogopenloghistory.ui

DISTFILES += \
    HMI_Syle.css

RESOURCES += \
    resource.qrc

unix:!macx: LIBS += -L/usr/lib/ -llog4qt

INCLUDEPATH += /include
DEPENDPATH += /include

unix:!macx: LIBS += -L/usr/share/RMAP/lib/ -lqmapcontrol

INCLUDEPATH += /usr/share/RMAP/include
DEPENDPATH += /usr/share/RMAP/include

unix:!macx: LIBS += -L/usr/local/lib/ -lmosquittopp

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

