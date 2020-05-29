#ifndef FRAMELEFT_H
#define FRAMELEFT_H

#include <QFrame>

#include "dialogradar.h"
#include "traildialog.h"
#include "dialogiff.h"
#include "dialogadsb.h"
#include "dialoglogging.h"

namespace Ui {
class FrameLeft;
}

class FrameLeft : public QFrame
{
    Q_OBJECT

public:
    explicit FrameLeft(QWidget *parent = 0);
    ~FrameLeft();

    void setRangeRings(qreal range_ring);

signals:
    void signal_exit();
    void signal_mapChange(quint8 id, quint8 val);

private slots:
    void on_checkBoxShowRing_clicked(bool checked);

    void on_checkBoxShowMap_clicked(bool checked);

    void on_comboBoxMapMode_activated(int index);

    void on_checkBoxShowHM_clicked(bool checked);

    void on_checkBoxShowCompass_clicked(bool checked);

    void on_pushButtonSetRadar_clicked();

    void on_pushButtonSetTrail_clicked();

    void on_pushButtonSetIFF_clicked();

    void on_pushButtonSetADSB_clicked();

    void on_pushButtonARPA_2_clicked();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Ui::FrameLeft *ui;

    DialogRadar *dRadar;
    TrailDialog *dTrail;
    DialogIFF *dIFF;
    DialogADSB *dADSB;
    DialogLogging *dLog;
};

#endif // FRAMELEFT_H
