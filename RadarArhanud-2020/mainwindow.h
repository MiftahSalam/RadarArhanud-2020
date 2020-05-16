#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "radarscene.h"
#include "echo/radar.h"

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
    void signal_arpa_target_param(int id, double rng, double brn, double lat, double lon, double spd, double crs);

private slots:
    void trigger_ReqDelTrack(int id);
    void trigger_rangeChange(int rng);
    void trigger_reqCreateArpa(QPointF position);
    void trigger_DrawSpoke(int, u_int8_t*, size_t);
    void trigger_forceExit();
    void timeOut();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    RI *m_ri;
    RA *m_ra;
    RadarScene *scene;

    int m_range_meters;
    int m_range_pixel;
    int cur_arpa_id_count;

    void calculateRadarScale();
};

#endif // MAINWINDOW_H
