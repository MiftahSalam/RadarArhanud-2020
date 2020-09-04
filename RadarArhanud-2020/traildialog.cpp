#include "traildialog.h"
#include "ui_traildialog.h"
#include <radarengine_global.h>

TrailDialog::TrailDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrailDialog)
{
    ui->setupUi(this);

    ui->checkBoxTrailOff->setChecked(trail_settings.enable);
    ui->comboBoxTargetTail->setCurrentIndex(trail_settings.trail);
}

TrailDialog::~TrailDialog()
{
    delete ui;
}

void TrailDialog::on_comboBoxTargetTail_activated(int index)
{
    trail_settings.trail = index;
}

void TrailDialog::on_checkBoxTrailOff_clicked(bool checked)
{
    trail_settings.enable = checked;
}

void TrailDialog::on_pushButtonClearTrail_clicked()
{
    emit signal_clearTrailReq();
}
