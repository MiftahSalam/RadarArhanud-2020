#include "dialogradarspec.h"
#include "ui_dialogradarspec.h"

DialogRadarSpec::DialogRadarSpec(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRadarSpec)
{
    ui->setupUi(this);
}

DialogRadarSpec::~DialogRadarSpec()
{
    delete ui;
}
