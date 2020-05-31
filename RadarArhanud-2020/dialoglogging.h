#ifndef DIALOGLOGGING_H
#define DIALOGLOGGING_H

#include <QDialog>
#include <QStandardItemModel>

#include "dialoglogdetail.h"
#include "dialogopenloghistory.h"

namespace Ui {
class DialogLogging;
}

class DialogLogging : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLogging(QWidget *parent = 0);
    ~DialogLogging();

    void insertLog(const QString &msg);

private slots:
    void on_pushButtonLatestLog_clicked();

    void on_lineEditFilter_textChanged(const QString &arg1);

    void on_comboBoxFilter_currentIndexChanged(int index);

    void on_tableViewLog_doubleClicked(const QModelIndex &index);

    void on_pushButtonOpenLog_clicked();

    void trigger_OpenFileLogHist(QString file);

private:
    Ui::DialogLogging *ui;
    DialogLogDetail *logDetail;
    DialogOpenLogHistory *logHist;

    QStandardItemModel *logViewModel;
    QString curFilename;

    int curFilterColumn;
    bool curLogMode;

    void openAndReadFile(const QString &filename);

};

#endif // DIALOGLOGGING_H
