#ifndef DIALOGLOGDETAIL_H
#define DIALOGLOGDETAIL_H

#include <QDialog>

namespace Ui {
class DialogLogDetail;
}

class DialogLogDetail : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLogDetail(QWidget *parent = 0);
    ~DialogLogDetail();

    void setDate(const QString date);
    void setType(const QString type);
    void setSource(const QString source);
    void setMessage(const QString msg);
//    void setDetail(const QString source);

private:
    Ui::DialogLogDetail *ui;
};

#endif // DIALOGLOGDETAIL_H
