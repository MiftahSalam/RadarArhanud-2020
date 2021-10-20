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

    int getNavStatus() const;

signals:
    void signal_request_del_track(int);
    void signal_request_del_adsb_track(quint32);
    void signal_update_track_num(int id,int number);
    void signal_target_select_update(
            QString id,
            QString tn,
            QString rng,
            QString brn,
            QString lat,
            QString lon,
            QString spd,
            QString crs,
            QString alt
            );

private slots:
    void timeoutUpdate();
    void trigger_target_selected(int tn,bool selected);
    void trigger_target_update(quint32 icao,
                                  double rng,
                                  double brn,
                                  double lat,
                                  double lon,
                                  double spd,
                                  double crs,
                                  double alt,
                                  QString call_sign,
                                  QString country, bool selected, quint8 identity, QString squawk, quint8 cat);
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
    QStandardItemModel *trackModel;
    QHash<quint32,quint64> target_time_tag_list;
    QTimer timer;
    qtmosq *m_mqtt;
    QString append_data_osd;
    QString cur_selected_track;

    int dataCount_mqtt_track;
    quint8 no_osd_count;
    bool osd_col_normal;

    void insertTrackList(quint32 icao,
                        double rng,
                        double brn,
                        double lat,
                        double lon,
                        double spd,
                        double crs,
                        double alt,
                        QString call_sign,
                        QString country, quint8 identity, QString squawk, quint8 cat);

    QString int2Cat(const quint8 cat);
    QString int2Identity(const quint8 ident);
};

#endif // FRAMEBOTTOM_H
