#ifndef DIALOGLOGGING_H
#define DIALOGLOGGING_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class DialogLogging;
}

class DialogLogging : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLogging(QWidget *parent = 0);
    ~DialogLogging();

private:
    Ui::DialogLogging *ui;

    QStandardItemModel *logViewModel;

};

#endif // DIALOGLOGGING_H
