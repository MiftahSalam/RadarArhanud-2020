#ifndef DIALOGIFF_H
#define DIALOGIFF_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStandardItem>

#include "iff-arhnd/iff/iff.h"

namespace Ui {
class DialogIFF;
}

class DialogIFF : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogIFF(QWidget *parent = 0);
    ~DialogIFF();

    QString getStatus() const;
    void updateLatLongHdt();
    
signals:
    void signal_settingChange();

private slots:
    void trigger_ackDataHandle(quint8 msg_type, quint8 msg_id, quint8 state);
    void trigger_BITDataHandle(IFFArhnd::BITResult bit);
    void trigger_InstallationResponse(QString data);

    void on_pushButtonApply_clicked();

    void on_pushButtonApplyFlightID_clicked();

    void on_pushButtonApplyOperating_clicked();

    void on_pushButtonApplyTrgReqSend_clicked();

    void on_pushButtonOperatinalApply_clicked();

    void on_comboBoxOperationalIntrrMode_activated(const QString &arg1);

    void on_pushButtonBITSendReq_clicked();

    void on_pushButtonSettingsInstallation_clicked();

    void on_pushButtonApplyOperatingUpdateGps_clicked();

    void on_pushButtonOpFriendListRemove_clicked();

    void on_pushButtonOpFriendListAdd_clicked();

private:
    Ui::DialogIFF *ui;
    QStandardItemModel *codeListModel;

    IFFArhnd::IFFService *iff;

    bool toggleColor;
    quint8 input_required_count;
};

#endif // DIALOGIFF_H
