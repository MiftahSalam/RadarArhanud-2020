#ifndef DIALOGOPENLOGHISTORY_H
#define DIALOGOPENLOGHISTORY_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class DialogOpenLogHistory;
}

class DialogOpenLogHistory : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOpenLogHistory(QWidget *parent = 0);
    ~DialogOpenLogHistory();

    void setFileList(const QStringList &files);

signals:
    void signal_openFile(QString file);

private slots:
    void on_buttonBox_accepted();

    void on_listView_doubleClicked(const QModelIndex &index);

private:
    Ui::DialogOpenLogHistory *ui;

    QStringListModel *logListModel;
};

#endif // DIALOGOPENLOGHISTORY_H
