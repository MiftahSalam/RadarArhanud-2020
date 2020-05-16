#ifndef DIALOGADSB_H
#define DIALOGADSB_H

#include <QDialog>

namespace Ui {
class DialogADSB;
}

class DialogADSB : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogADSB(QWidget *parent = 0);
    ~DialogADSB();
    
signals:
    void signal_settingChange();

private slots:
    void on_pushButtonApply_clicked();

private:
    Ui::DialogADSB *ui;
};

#endif // DIALOGADSB_H
