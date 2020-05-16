#ifndef FRAMEBOTTOM_H
#define FRAMEBOTTOM_H

#include <QFrame>
#include <QTimer>
#include <QStandardItemModel>

class RI;

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

private slots:
    void timeoutUpdate();
    void trigger_arpa_target_param(int id, double rng, double brn, double lat, double lon, double spd, double crs);


    void on_pushButtonDelSel_clicked();

    void on_pushButtonDelAll_clicked();

private:
    Ui::FrameBottom *ui;
    QStandardItemModel *arpaModel;

    QHash<int,quint64> target_arpa_time_tag_list;

    QTimer timer;

    void insertArpaList(int id, double lat, double lon, double spd, double crs, double rng, double brn);

};

#endif // FRAMEBOTTOM_H
