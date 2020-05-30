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

    void insertLog(const QString &msg);

private slots:
    void on_pushButtonLatestLog_clicked();

    void on_lineEditFilter_textChanged(const QString &arg1);

    void on_comboBoxFilter_currentIndexChanged(int index);

private:
    Ui::DialogLogging *ui;

    QStandardItemModel *logViewModel;
    QString curFilename;

    int curFilterColumn;
    bool curLogMode;

    void openAndReadFile(const QString &filename);

};

#endif // DIALOGLOGGING_H
