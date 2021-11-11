#-------------------------------------------------
#
# Project created by QtCreator 2020-03-12T14:36:06
#
#-------------------------------------------------

QT       += core gui opengl network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadarArhanud-2021
TEMPLATE = app

DEFINES += UBUNTU16

SOURCES += main.cpp\
    dialogbit.cpp \
    dialogradarspec.cpp \
        mainwindow.cpp \
    frameleft.cpp \
    framebottom.cpp \
    radargraphicview.cpp \
    radarscene.cpp \
    arpatrackitem.cpp \
    ifftrackitem.cpp \
    radarsceneitems.cpp \
    adsbtrackitem.cpp \
    dialogradar.cpp \
    trackmanager.cpp \
    traildialog.cpp \
    dialogiff.cpp \
    dialogadsb.cpp \
    qtmosq.cpp \
    dialoglogging.cpp \
    dialoglogdetail.cpp \
    dialogopenloghistory.cpp \
    dialogselectedtrack.cpp

HEADERS  += mainwindow.h \
    dialogbit.h \
    dialogradarspec.h \
    frameleft.h \
    framebottom.h \
    radargraphicview.h \
    radarscene.h \
    arpatrackitem.h \
    ifftrackitem.h \
    radarsceneitems.h \
    adsbtrackitem.h \
    dialogradar.h \
    trackmanager.h \
    traildialog.h \
    dialogiff.h \
    dialogadsb.h \
    qtmosq.h \
    dialoglogging.h \
    dialoglogdetail.h \
    dialogopenloghistory.h \
    dialogselectedtrack.h

FORMS    += mainwindow.ui \
    dialogbit.ui \
    dialogradarspec.ui \
    frameleft.ui \
    framebottom.ui \
    dialogradar.ui \
    traildialog.ui \
    dialogiff.ui \
    dialogadsb.ui \
    dialoglogging.ui \
    dialoglogdetail.ui \
    dialogopenloghistory.ui \
    dialogselectedtrack.ui

DISTFILES += \
    HMI_Syle.css \
    log4qt.properties

RESOURCES += \
    resource.qrc

unix:!macx: LIBS += -L/usr/lib/ -llog4qt

INCLUDEPATH += /include
DEPENDPATH += /include

#unix:!macx: LIBS += -L/usr/lib/Crypto/ -lCrypto

#INCLUDEPATH += /usr/include/Crypto
#DEPENDPATH += /usr/include/Crypto

unix:!macx: LIBS += -L/usr/lib/RMAP/ -lqmapcontrol

INCLUDEPATH += /usr/include/RMAP/
DEPENDPATH += /usr/include/RMAP/

unix:!macx: LIBS += -L/usr/local/lib/ -lmosquittopp

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include


unix:!macx: LIBS += -L/usr/lib/ -ladsb-arhnd

INCLUDEPATH += /usr/include/adsb-arhnd
DEPENDPATH += /usr/include/adsb-arhnd

unix:!macx: LIBS += -L/usr/lib/ -lRadarEngine-arnd

INCLUDEPATH += /usr/include/RadarEngine
DEPENDPATH += /usr/include/RadarEngine

unix:!macx: PRE_TARGETDEPS += /usr/lib/libRadarEngine-arnd.a

unix:!macx: LIBS += -L$$PWD/../../../../../../../../../../usr/lib/Crypto/ -lCrypto

INCLUDEPATH += $$PWD/../../../../../../../../../../usr/include/Crypto
DEPENDPATH += $$PWD/../../../../../../../../../../usr/include/Crypto


unix:!macx: LIBS += -L/usr/lib/ -liff-arhnd

INCLUDEPATH += /usr/include/iff-arhnd
DEPENDPATH += /usr/include/iff-arhnd

INCLUDEPATH += /usr/include/stream-arhnd
DEPENDPATH += /usr/include/stream-arhnd
