#include "framebottom.h"
#include "ui_framebottom.h"
#include "echo/radar.h"

FrameBottom::FrameBottom(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameBottom)
{
    ui->setupUi(this);

    connect(&timer,SIGNAL(timeout()),this,SLOT(timeoutUpdate()));
    timer.start(1000);

}

void FrameBottom::timeoutUpdate()
{
    qDebug()<<Q_FUNC_INFO;
}

FrameBottom::~FrameBottom()
{
    delete ui;
}
