#include "frameleft.h"
#include "ui_frameleft.h"

FrameLeft::FrameLeft(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameLeft)
{
    ui->setupUi(this);
}

FrameLeft::~FrameLeft()
{
    delete ui;
}
