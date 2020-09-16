#ifndef DIALOGRADAR_H
#define DIALOGRADAR_H

#include <QDialog>

namespace Ui {
class DialogRadar;
}

class DialogRadar : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogRadar(QWidget *parent = 0);
    ~DialogRadar();
    
signals:
    void signal_settingChange();

private slots:
    void on_pushButtonApply_clicked();

    void on_checkBoxShowTrackData_clicked(bool checked);

private:
    Ui::DialogRadar *ui;
};

#endif // DIALOGRADAR_H
