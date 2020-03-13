#ifndef FRAMELEFT_H
#define FRAMELEFT_H

#include <QFrame>

namespace Ui {
class FrameLeft;
}

class FrameLeft : public QFrame
{
    Q_OBJECT

public:
    explicit FrameLeft(QWidget *parent = 0);
    ~FrameLeft();

private:
    Ui::FrameLeft *ui;
};

#endif // FRAMELEFT_H
