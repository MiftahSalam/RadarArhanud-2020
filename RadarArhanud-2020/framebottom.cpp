#include "framebottom.h"
#include "ui_framebottom.h"

FrameBottom::FrameBottom(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameBottom)
{
    ui->setupUi(this);

}

FrameBottom::~FrameBottom()
{
    delete ui;
}
