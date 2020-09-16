#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <log4qt/signalappender.h>

#include "radarscene.h"
#include "adsb/adsbstream.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void signal_reqRangeChange(int range);
    void signal_trueLog(QString msg);
    void signal_arpa_target_param(int id, double rng, double brn, double lat, double lon, double spd, double crs);
    void signal_adsb_target_param(quint32 icao,
                                  double rng,
                                  double brn,
                                  double lat,
                                  double lon,
                                  double spd,
                                  double crs,
                                  double alt,
                                  QString call_sign,
                                  QString country);

private slots:
    void trigger_positionChange();
    void trigger_ReqDelTrack(int id);
    void trigger_rangeChange();
    void trigger_radarFeedbackRangeChange(int);
    void trigger_reqCreateArpa(QPointF position);
    void trigger_reqUpdateADSB(QByteArray data);
    void trigger_ReqDelAdsb(quint32 icao);
    void trigger_DrawSpoke(int, u_int8_t*, size_t);
    void trigger_forceExit();
    void trigger_logEvent(QString msg);
    void timeOut();

    void initADSB();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    RadarEngineARND::RadarEngine *m_ri;
    AdsbArhnd::ADSBStream *adsb;
    QSet<quint32> adsb_list;

    RadarScene *scene;

    Log4Qt::SignalAppender *logEvent;

    double m_range_meters,m_radar_range,m_range_to_send;
    int m_range_pixel;
    int cur_arpa_id_count;
    bool first;

    void calculateRadarScale();
};

#endif // MAINWINDOW_H
