#ifndef FRAMEBOTTOM_H
#define FRAMEBOTTOM_H

#include <QFrame>

namespace Ui {
class FrameBottom;
}

class FrameBottom : public QFrame
{
    Q_OBJECT

public:
    explicit FrameBottom(QWidget *parent = 0);
    ~FrameBottom();

private:
    Ui::FrameBottom *ui;
};

#endif // FRAMEBOTTOM_H
