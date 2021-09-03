#ifndef DIALOGIFF_H
#define DIALOGIFF_H

#include <QDialog>

#include "iff-arhnd/iff/iff.h"

namespace Ui {
class DialogIFF;
}

class DialogIFF : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogIFF(QWidget *parent = 0);
    ~DialogIFF();
    
signals:
    void signal_settingChange();

private slots:
    void on_pushButtonApply_clicked();

private:
    Ui::DialogIFF *ui;

    IFFArhnd::IFFService *iff;
};

#endif // DIALOGIFF_H
