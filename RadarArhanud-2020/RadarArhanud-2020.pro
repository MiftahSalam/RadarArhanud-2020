#-------------------------------------------------
#
# Project created by QtCreator 2020-03-12T14:36:06
#
#-------------------------------------------------

QT       += core gui opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadarArhanud-2020
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    frameleft.cpp \
    framebottom.cpp \
    radargraphicview.cpp \
    radarscene.cpp \
    arpatrackitem.cpp \
    echo/radar.cpp \
    ifftrackitem.cpp \
    radarsceneitems.cpp

HEADERS  += mainwindow.h \
    frameleft.h \
    framebottom.h \
    radargraphicview.h \
    radarscene.h \
    arpatrackitem.h \
    echo/radar.h \
    echo/radar_global.h \
    ifftrackitem.h \
    radarsceneitems.h

FORMS    += mainwindow.ui \
    frameleft.ui \
    framebottom.ui

DISTFILES += \
    HMI_Syle.css

RESOURCES += \
    resource.qrc

unix:!macx: LIBS += -L/usr/share/RMAP/lib/ -lqmapcontrol

INCLUDEPATH += /usr/share/RMAP/include
DEPENDPATH += /usr/share/RMAP/include
