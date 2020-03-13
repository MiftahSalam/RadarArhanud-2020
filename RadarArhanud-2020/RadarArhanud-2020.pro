#-------------------------------------------------
#
# Project created by QtCreator 2020-03-12T14:36:06
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadarArhanud-2020
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    frameleft.cpp \
    framebottom.cpp \
    radarwidget.cpp

HEADERS  += mainwindow.h \
    frameleft.h \
    framebottom.h \
    radarwidget.h

FORMS    += mainwindow.ui \
    frameleft.ui \
    framebottom.ui

DISTFILES += \
    HMI_Syle.css

RESOURCES += \
    resource.qrc
