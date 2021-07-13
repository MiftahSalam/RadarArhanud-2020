#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <log4qt/signalappender.h>

#include "radarscene.h"
#include "adsb/adsbstream.h"
#include "dialogselectedtrack.h"
#include "trackmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    RadarEngineARND::RadarEngine *m_ri,*m_ri1;

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void signal_reqRangeChange(int range);
    void signal_trueLog(QString msg);
    void signal_target_param(quint32 id,
                             double rng,
                             double brn,
                             double lat,
                             double lon,
                             double spd,
                             double crs,
                             double alt,
                             QString call_sign,
                             QString country,
                             bool selected
                             );

private slots:
    void trigger_opModeChange(bool checked);
    void trigger_positionChange();
    void trigger_ReqDelTrack(int id);
    void trigger_rangeChange();
    void trigger_radarFeedbackRangeChange(int);
    void trigger_reqCreateArpa(QPointF position);
    void trigger_reqUpdateADSB(QByteArray data);
    void trigger_ReqDelAdsb(quint32 icao);
    void trigger_updateTrackNumber(int id, int number);
    void trigger_DrawSpoke(int, u_int8_t*, size_t);
    void trigger_DrawSpoke1(int, u_int8_t*, size_t);
    void trigger_forceExit();
    void trigger_logEvent(QString msg);
    void timeOut();

    void initADSB();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    DialogSelectedTrack *trackDialog;

    TrackManager *m_tm;
    AdsbArhnd::ADSBStream *adsb;
    QSet<quint32> adsb_list;

    RadarScene *scene;

    Log4Qt::SignalAppender *logEvent;

    RadarState curState;
    double m_range_meters,m_radar_range,m_range_to_send;
    int m_range_pixel;
    int cur_arpa_id_count;
    bool first;

    void calculateRadarScale();
};

#endif // MAINWINDOW_H
