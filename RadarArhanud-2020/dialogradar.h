#ifndef DIALOGRADAR_H
#define DIALOGRADAR_H

#include <QDialog>
#include <QResizeEvent>

namespace Ui {
class DialogRadar;
}

class DialogRadar : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogRadar(QWidget *parent = 0);
    ~DialogRadar();

    void trigger_fixRangeMode(bool fix);
    
signals:
    void signal_settingChange();

protected:
    void resizeEvent(QResizeEvent *evt);

private slots:
    void on_pushButtonApply_clicked();

    void on_checkBoxShowTrackData_clicked(bool checked);

    void on_pushButtonApply_2_clicked();

private:
    Ui::DialogRadar *ui;
    int halfHeight, fullHeight;
};

#endif // DIALOGRADAR_H
