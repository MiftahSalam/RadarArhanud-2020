#ifndef DIALOGSELECTEDTRACK_H
#define DIALOGSELECTEDTRACK_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>

namespace Ui {
class DialogSelectedTrack;
}

class DialogSelectedTrack : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectedTrack(QWidget *parent = 0);
    ~DialogSelectedTrack();

    void trigger_target_select_update(
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
    void timerTimeout();

private:
    Ui::DialogSelectedTrack *ui;
    QDateTime cur_update_time;
    QTimer *timer;
};

#endif // DIALOGSELECTEDTRACK_H
