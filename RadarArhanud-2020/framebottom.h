#ifndef FRAMEBOTTOM_H
#define FRAMEBOTTOM_H

#include <QFrame>
#include <QTimer>
#include <QStandardItemModel>

#include "adsb/adsb.h"
#include "qtmosq.h"

class RadarEngine;

namespace Ui {
class FrameBottom;
}

class FrameBottom : public QFrame
{
    Q_OBJECT

public:
    explicit FrameBottom(QWidget *parent = 0);
    ~FrameBottom();

signals:
    void signal_request_del_track(int);
    void signal_request_del_adsb_track(quint32);

private slots:
    void timeoutUpdate();
    void trigger_arpa_target_param(int id, double rng, double brn, double lat, double lon, double spd, double crs);
    void trigger_adsb_target_update(quint32 icao,
                                  double rng,
                                  double brn,
                                  double lat,
                                  double lon,
                                  double spd,
                                  double crs,
                                  double alt,
                                  QString call_sign,
                                  QString country);
    void trigger_OSD_received(QString msg);
    void trigger_OSD_connected();
    void trigger_OSD_disconnected();

    void on_pushButtonDelSel_clicked();

    void on_pushButtonDelAll_clicked();

    void on_pushButtonApply_clicked();

    void on_checkBoxGPS_clicked(bool checked);

    void on_checkBoxHDG_clicked(bool checked);

private:
    Ui::FrameBottom *ui;
    QStandardItemModel *arpaModel;
    QStandardItemModel *adsbModel;

    QHash<int,quint64> target_arpa_time_tag_list;
    QHash<quint32,quint64> target_adsb_time_tag_list;

    QTimer timer;

    qtmosq *m_mqtt;

    int dataCount_mqtt_arpa,dataCount_mqtt_adsb;
    quint8 no_osd_count;
    bool osd_col_normal;

    void insertArpaList(int id, double lat, double lon, double spd, double crs, double rng, double brn);
    void insertADSBList(quint32 icao,
                        double rng,
                        double brn,
                        double lat,
                        double lon,
                        double spd,
                        double crs,
                        double alt,
                        QString call_sign,
                        QString country);

};

#endif // FRAMEBOTTOM_H
