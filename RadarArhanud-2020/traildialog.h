#ifndef TRAILDIALOG_H
#define TRAILDIALOG_H

#include <QDialog>

namespace Ui {
class TrailDialog;
}

class TrailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrailDialog(QWidget *parent = 0);
    ~TrailDialog();

signals:
    void signal_clearTrailReq();

private slots:
    void on_comboBoxTargetTail_activated(int index);

    void on_checkBoxTrailOff_clicked(bool checked);

    void on_pushButtonClearTrail_clicked();

private:
    Ui::TrailDialog *ui;
};

#endif // TRAILDIALOG_H
