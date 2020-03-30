#ifndef FRAMEBOTTOM_H
#define FRAMEBOTTOM_H

#include <QFrame>
#include <QTimer>
#include <QStandardItemModel>

class RI;

namespace Ui {
class FrameBottom;
}

class FrameBottom : public QFrame
{
    Q_OBJECT

public:
    explicit FrameBottom(QWidget *parent = 0);
    ~FrameBottom();

    void setRadarInfo(RI *ri) { if(m_ri != NULL) m_ri = ri; }

private slots:
    void timeoutUpdate();

private:
    Ui::FrameBottom *ui;
    QStandardItemModel *arpaModel;

    RI *m_ri;
    QTimer timer;
};

#endif // FRAMEBOTTOM_H
