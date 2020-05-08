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

private slots:
    void trigger_rangeChange(int rng);
    void trigger_reqCreateArpa(QPointF position);
    void trigger_DrawSpoke(int,int,u_int8_t*,size_t);

private:
    Ui::MainWindow *ui;

    RI *m_ri;
    RA *m_ra;
    RadarScene *scene;

    int m_range_meters;
    int m_range_pixel;

    void calculateRadarScale();
};

#endif // MAINWINDOW_H
