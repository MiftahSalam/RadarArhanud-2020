#include "dialogiff.h"
#include "ui_dialogiff.h"
#include <radarengine_global.h>

#include <QIntValidator>
#include <QMessageBox>

DialogIFF::DialogIFF(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogIFF)
{
    ui->setupUi(this);
    ui->tabWidget_4->hide(); //interrogator mode s settings
    ui->groupBox_7->hide(); //track output settings
    ui->tabOperationalWhisper->setEnabled(false);
    ui->tabOpCon2->setEnabled(false);

    ui->lineEditOPPortCon2->setValidator(new QIntValidator(0,255,ui->lineEditOPPortCon2));
    ui->lineEditOPPortCon1->setValidator(new QIntValidator(300,65536,ui->lineEditOPPortCon1));

    qDebug()<<Q_FUNC_INFO<<iff_settings.ip1<<friendListCode<<hostileListCode;
    ui->lineEditOPIPCon1->setText(iff_settings.ip1);
    ui->lineEditOPPortCon1->setText(QString::number(iff_settings.port1));

    QStandardItem *item1 = new QStandardItem("Code");

    codeFriendListModel = new QStandardItemModel(this);
    codeFriendListModel->setColumnCount(1);
    codeFriendListModel->setHorizontalHeaderItem(0,item1);

    codeHostileListModel = new QStandardItemModel(this);
    codeHostileListModel->setColumnCount(1);
    codeHostileListModel->setHorizontalHeaderItem(0,item1);

    foreach (QString code, friendListCode)
    {
        qDebug()<<Q_FUNC_INFO<<"friend"<<code;
        codeFriendListModel->insertRow(codeFriendListModel->rowCount(), QModelIndex());
        codeFriendListModel->setData(codeFriendListModel->index(codeFriendListModel->rowCount()-1,0,QModelIndex()),code);
        codeFriendListModel->item(codeFriendListModel->rowCount()-1,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        friendCodeReplyList.insert(code,QDateTime::currentSecsSinceEpoch());
    }
    foreach (QString code, hostileListCode)
    {
        qDebug()<<Q_FUNC_INFO<<"hostileListCode"<<code;
        codeHostileListModel->insertRow(codeHostileListModel->rowCount(), QModelIndex());
        codeHostileListModel->setData(codeHostileListModel->index(codeHostileListModel->rowCount()-1,0,QModelIndex()),code);
        codeHostileListModel->item(codeHostileListModel->rowCount()-1,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    }

    ui->tableViewOpFriendList->setModel(codeFriendListModel);
    ui->tableViewOpFriendList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableViewOpHostileList->setModel(codeHostileListModel);
    ui->tableViewOpHostileList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    StreamArhnd::StreamSettings iffSettingIn, iffSettingOut;

    iffSettingIn.config = iff_settings.ip2+";"+QString::number(iff_settings.port2);
    iffSettingIn.mode = StreamArhnd::In;
    iffSettingIn.type = (StreamArhnd::StreamType)1; //tcp
    iffSettingOut.config = iff_settings.ip1+";"+QString::number(iff_settings.port1);
    iffSettingOut.mode = StreamArhnd::InOut;
    iffSettingOut.type = (StreamArhnd::StreamType)1; //tcp
    toggleColor = true;
    input_required_count = 0;

    iff = IFFArhnd::IFFService::getIntance(iffSettingIn, iffSettingOut);
    connect(iff,&IFFArhnd::IFFService::signal_ack_message,this,&DialogIFF::trigger_ackDataHandle);
    connect(iff,&IFFArhnd::IFFService::signal_BIT,this,&DialogIFF::trigger_BITDataHandle);
    connect(iff,&IFFArhnd::IFFService::signal_InstallationResponse,this,&DialogIFF::trigger_InstallationResponse);
    QTimer::singleShot(5000,[=](){
        iff->dataReq("1");
        qDebug()<<Q_FUNC_INFO<<"request installation";
    });
    iff->setLatLonHdt(currentOwnShipLat, currentOwnShipLon, currentHeading);
}

void DialogIFF::updateLatLongHdt()
{
    iff->setLatLonHdt(currentOwnShipLat, currentOwnShipLon, currentHeading);
}
QString DialogIFF::getStatus() const
{
    QString status;
    if(ui->labelStatusConnection->text() == "Online")
    {
        if(ui->labelOperationalStatusError->text() == "No Error") status = ui->labelOperationalStatusMode->text();
        else status = ui->labelOperationalStatusError->text();
    }
    else status = "Offline";


    QHashIterator<QString,qint64> i(friendCodeReplyList);
    qint64 now = QDateTime::currentSecsSinceEpoch();
    const qint64 delta = 30;
    QList<QStandardItem *> listTarget;

    while (i.hasNext())
    {
        i.next();
        if(now-i.value() > delta)
        {
            qDebug()<<Q_FUNC_INFO<<"no reply track. delta"<<delta<<"id"<<i.key();

            listTarget = codeFriendListModel->findItems(i.key(),Qt::MatchExactly,0);
            if(listTarget.size() > 0)
            {
                int row = listTarget.at(0)->row();

                if(row >= 0)
                {
                    if(codeFriendListModel->item(row,0)->background() != Qt::NoBrush)
                        codeFriendListModel->item(row,0)->setBackground(QBrush(Qt::NoBrush));
                }
            }
        }
    }


    return status;
}

void DialogIFF::trigger_interrogateReply(QString code)
{
    QList<QStandardItem *> listTarget;

    listTarget = codeFriendListModel->findItems(code,Qt::MatchExactly,0);

    if(listTarget.size() > 0)
    {
        int row = listTarget.at(0)->row();

        if(row >= 0)
        {
            codeFriendListModel->item(row,0)->setBackground(QBrush(Qt::green,Qt::SolidPattern));
            friendCodeReplyList.insert(code,QDateTime::currentSecsSinceEpoch());
        }
    }


}
void DialogIFF::trigger_InstallationResponse(QString source)
{
    QStringList source_list = source.split(";");

    qDebug()<<Q_FUNC_INFO<<"message source_list"<<source_list;

    if(source_list.size() == 7)
    {
        ui->lineEditSettingSetupInstlICAO->setText(source_list.at(0));
        ui->lineEditSettingSetupInstlReg->setText(source_list.at(1));
        ui->lineEditSettingSetupInstlIP->setText(source_list.at(2));
        ui->lineEditSettingSetupInstlNet->setText(source_list.at(3));
        ui->lineEditSettingSetupInstlPort->setText(source_list.at(4));
        ui->comboBoxSettingSetupCom0->setCurrentIndex(source_list.at(5).toInt());
        ui->comboBoxSettingSetupCom1->setCurrentIndex(source_list.at(6).toInt());

    }
    else qDebug()<<Q_FUNC_INFO<<"invalid message source"<<source;

}
void DialogIFF::trigger_BITDataHandle(IFFArhnd::BITResult bit)
{
    qDebug()<<Q_FUNC_INFO<<"input_power_in_range"<<bit.input_power_in_range;
    qDebug()<<Q_FUNC_INFO<<"icao_address_valid"<<bit.icao_address_valid;
    qDebug()<<Q_FUNC_INFO<<"gps_position_valid"<<bit.gps_position_valid;
    qDebug()<<Q_FUNC_INFO<<"gps_pps_valid"<<bit.gps_pps_valid;
    qDebug()<<Q_FUNC_INFO<<"temperature_in_range"<<bit.temperature_in_range;
    qDebug()<<Q_FUNC_INFO<<"squitter_rate_valid"<<bit.squitter_rate_valid;
    qDebug()<<Q_FUNC_INFO<<"transmot_rate_valid"<<bit.transmot_rate_valid;
    qDebug()<<Q_FUNC_INFO<<"rf_loopback_pass"<<bit.rf_loopback_pass;
    qDebug()<<Q_FUNC_INFO<<"power_53_valid"<<bit.power_53_valid;
    qDebug()<<Q_FUNC_INFO<<"adc_pass"<<bit.adc_pass;
    qDebug()<<Q_FUNC_INFO<<"pressure_tranducer_ready"<<bit.pressure_tranducer_ready;
    qDebug()<<Q_FUNC_INFO<<"fpga_pass"<<bit.fpga_pass;
    qDebug()<<Q_FUNC_INFO<<"rx_osc_locked"<<bit.rx_osc_locked;
    qDebug()<<Q_FUNC_INFO<<"tx_osc_locked"<<bit.tx_osc_locked;
    qDebug()<<Q_FUNC_INFO<<"power_on_pass"<<bit.power_on_pass;
    qDebug()<<Q_FUNC_INFO<<"continous_test_pass"<<bit.continous_test_pass;
    qDebug()<<Q_FUNC_INFO<<"processor_test_pass"<<bit.processor_test_pass;
    qDebug()<<Q_FUNC_INFO<<"flask_image_test_pass"<<bit.flask_image_test_pass;
    qDebug()<<Q_FUNC_INFO<<"memory_test_pass"<<bit.memory_test_pass;
    qDebug()<<Q_FUNC_INFO<<"calibrated_pass"<<bit.calibrated_pass;

    if(bit.input_power_in_range)
    {
        ui->labelPwrIn->setText("OK");
        ui->labelPwrIn->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelPwrIn->setText("Fail");
        ui->labelPwrIn->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.icao_address_valid)
    {
        ui->labelICAO->setText("OK");
        ui->labelICAO->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelICAO->setText("Fail");
        ui->labelICAO->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.gps_position_valid)
    {
        ui->labelGPSIn->setText("OK");
        ui->labelGPSIn->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelGPSIn->setText("Fail");
        ui->labelGPSIn->setStyleSheet("background-color: rgb(164,0,0);");
    }
//    if(bit.gps_pps_valid)
//    {
//        ui->labelGPSPPS->setText("OK");
//        ui->labelGPSPPS->setStyleSheet("background-color: rgb(78, 154, 6);");
//    }
//    else
//    {
//        ui->labelGPSPPS->setText("Fail");
//        ui->labelGPSPPS->setStyleSheet("background-color: rgb(164,0,0);");
//    }
    if(bit.temperature_in_range)
    {
        ui->labelTemp->setText("OK");
        ui->labelTemp->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelTemp->setText("Fail");
        ui->labelTemp->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.squitter_rate_valid)
    {
        ui->labelSquitter->setText("OK");
        ui->labelSquitter->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelSquitter->setText("Fail");
        ui->labelSquitter->setStyleSheet("background-color: rgb(164,0,0);");
    }
//    if(bit.rf_loopback_pass)
//    {
//        ui->labelRFLoop->setText("OK");
//        ui->labelRFLoop->setStyleSheet("background-color: rgb(78, 154, 6);");
//    }
//    else
//    {
//        ui->labelRFLoop->setText("Fail");
//        ui->labelRFLoop->setStyleSheet("background-color: rgb(164,0,0);");
//    }
    if(bit.power_53_valid)
    {
        ui->label53VPwr->setText("OK");
        ui->label53VPwr->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->label53VPwr->setText("Fail");
        ui->label53VPwr->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.adc_pass)
    {
        ui->labelADC->setText("OK");
        ui->labelADC->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelADC->setText("Fail");
        ui->labelADC->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.pressure_tranducer_ready)
    {
        ui->labelPressure->setText("OK");
        ui->labelPressure->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelPressure->setText("Fail");
        ui->labelPressure->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.fpga_pass)
    {
        ui->labelFPGA->setText("OK");
        ui->labelFPGA->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelFPGA->setText("Fail");
        ui->labelFPGA->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.rx_osc_locked || bit.tx_osc_locked)
    {
        ui->labelOscillator->setText("OK");
        ui->labelOscillator->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelOscillator->setText("Fail");
        ui->labelOscillator->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.flask_image_test_pass)
    {
        ui->labelFlash->setText("OK");
        ui->labelFlash->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelFlash->setText("Fail");
        ui->labelFlash->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.memory_test_pass)
    {
        ui->labelMemory->setText("OK");
        ui->labelMemory->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelMemory->setText("Fail");
        ui->labelMemory->setStyleSheet("background-color: rgb(164,0,0);");
    }
    if(bit.calibrated_pass)
    {
        ui->labelCal->setText("OK");
        ui->labelCal->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelCal->setText("Fail");
        ui->labelCal->setStyleSheet("background-color: rgb(164,0,0);");
    }
//    if(bit.power_on_pass)
//    {
//        ui->labelPowerOn->setText("OK");
//        ui->labelPowerOn->setStyleSheet("background-color: rgb(78, 154, 6);");
//    }
//    else
//    {
//        ui->labelPowerOn->setText("Fail");
//        ui->labelPowerOn->setStyleSheet("background-color: rgb(164,0,0);");
//    }
    if(bit.processor_test_pass)
    {
        ui->labelProcessor->setText("OK");
        ui->labelProcessor->setStyleSheet("background-color: rgb(78, 154, 6);");
    }
    else
    {
        ui->labelProcessor->setText("Fail");
        ui->labelProcessor->setStyleSheet("background-color: rgb(164,0,0);");
    }

}

void DialogIFF::trigger_ackDataHandle(quint8 msg_type, quint8 msg_id, quint8 state)
{
    qDebug()<<Q_FUNC_INFO<<"msg_type"<<msg_type<<"msg_id"<<msg_id<<"state"<<state;

    if(msg_type != 0xFF && msg_id != 0xFF && state != 0xFF)
    {
        quint8 mode = 0xC0 & state;
        bool maint_mode = 0x10 & state;
        bool input_required = 0x02 & state;
//        bool system_error = 0x01 & state;

        qDebug()<<Q_FUNC_INFO<<"mode"<<mode<<"maint_mode"<<maint_mode<<"input_required"<<input_required/*<<"system_error"<<system_error*/;

        ui->groupBoxSettingsInstall->setEnabled(maint_mode);

        switch (mode) {
        case 0:
           ui->labelOperationalStatusMode->setText("Standby");
           ui->labelOperationalStatusMode->setStyleSheet("background-color: rgb(196, 160, 0);");
            break;
        case 0x40:
           ui->labelOperationalStatusMode->setText("On");
           ui->labelOperationalStatusMode->setStyleSheet("background-color: rgb(78, 154, 6);");
            break;
        case 0xC0:
           ui->labelOperationalStatusMode->setText("Alt");
           ui->labelOperationalStatusMode->setStyleSheet("background-color: rgb(78, 154, 6);");
            break;
        default:
            break;
        }

//        if(system_error)
//        {
//            ui->labelOperationalStatusError->setText("Sys Error");
//            ui->labelOperationalStatusError->setStyleSheet("background-color: rgb(164,0,0);");
//        }
        if(input_required)
        {
            input_required_count++;
            if(input_required_count > 10) input_required_count = 11;
            iff->setLatLonHdt(currentOwnShipLat, currentOwnShipLon, currentHeading);
        }
        else
            input_required_count = 0;

        if(input_required_count > 10)
        {
            ui->labelOperationalStatusError->setText("Input Required");
            ui->labelOperationalStatusError->setStyleSheet("background-color: rgb(164,0,0);");
        }
        else
        {
            ui->labelOperationalStatusError->setText("No Error");
            ui->labelOperationalStatusError->setStyleSheet("background-color: rgb(78, 154, 6);");
        }

        ui->labelStatusConnection->setText("Online");
        ui->labelStatusConnection->setStyleSheet("background-color: rgb(78, 154, 6);");
        /*
        if(toggleColor)
            ui->labelStatusConnection->setStyleSheet("background-color: rgb(78, 154, 6);");
        else
            ui->labelStatusConnection->setStyleSheet("color: rgb(255,255,255);");

        toggleColor ^= true;
        */
    }
    //tes sementara fix dulu
    /*
    else
    {
        ui->labelOperationalStatusError->setText("");
        ui->labelOperationalStatusMode->setText("Off");
        ui->labelStatusConnection->setText("Offline");
        ui->labelOperationalStatusError->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelOperationalStatusMode->setStyleSheet("background-color: rgb(164,0,0);");
        ui->labelStatusConnection->setStyleSheet("background-color: rgb(164,0,0);");
    }
    */
}
DialogIFF::~DialogIFF()
{
    delete ui;
}

void DialogIFF::on_pushButtonApplyFlightID_clicked()
{
    if(ui->lineEditFlightIDICAO->text().isEmpty())
        iff->setFlightID(ui->lineEditFlightIDICAO->text());
}

void DialogIFF::on_pushButtonApplyOperating_clicked()
{
    QString mode, es;

    switch (ui->comboBoxOpMode->currentIndex()) {
    case 0:
        mode = "STANDBY";
        break;
    case 1:
        mode = "ON";
        break;
    case 2:
        mode = "ALT";
        break;
    default:
        break;
    }

    es = "ES_DIS";
//    if(ui->checkBoxOpEnableES->isChecked()) es = "ES_EN";
//    else es = "ES_DIS";

    QStringList config = QStringList()<<ui->lineEditOpSquawk->text()
                                     <<mode+","+"PWR_STATE_DIS"+","+es
                                    <<"0"
                                   <<"0"
                                  <<"0"
                                 <<"0"
                                <<QString::number(currentHeading)
                                  ;
    iff->setOpMode(config.join(";"));
}

void DialogIFF::on_pushButtonApplyTrgReqSend_clicked()
{
    int report_req = 0;

    if(ui->checkBoxTrgReqPosSend->isChecked()) report_req += 0x01;
    if(ui->checkBoxTrgReqOwnPosSend->isChecked()) report_req += 0x80;

    QStringList config = QStringList()<<QString::number(ui->comboBoxTrgReqMode->currentIndex())
                                     <<QString::number(ui->comboBoxTrgReqTxPort->currentIndex())
                                    <<ui->spinBoxTrgReqParticipans->text()
                                   <<"03FE14"
                                  <<QString::number(report_req)
                                    ;
    iff->trackReq(config.join(";"));
}

void DialogIFF::on_pushButtonOperatinalApply_clicked()
{
    QString config;
    bool single_mode = ui->comboBoxOperationalIntrrMode->currentText() == "Single";

    if(single_mode)
    {
        bool ok;
        int icao = ui->lineEditOperationalIntrrICAO->text().toInt(&ok,16);

        if(ui->lineEditOperationalIntrrICAO->text().size() != 6)
        {
            QMessageBox::critical(this,"Input Error","Invalid ICAO input.\nICAO address must be 6 character",QMessageBox::Ok);
            return;
        }

        if(ok) config.append(ui->lineEditOperationalIntrrICAO->text()+";");
        else QMessageBox::critical(this,"Input Erro","Invalid ICAO input.\nInvalid address format",QMessageBox::Ok);

        if(ui->comboBoxIntrrReqType->currentText() == "Mode A") config.append("1;");
        else if(ui->comboBoxIntrrReqType->currentText() == "Mode C") config.append("2;");
        else if(ui->comboBoxIntrrReqType->currentText() == "Mode S") config.append("8;");

        config.append(ui->spinBoxOperationalIntrrAmpl->text()+";");
        config.append(QString::number(ui->comboBoxOperationalIntrrAntenna->currentIndex()));
    }
    else
    {
        if(ui->comboBoxIntrrReqType->currentText() == "Mode A") config.append("1;");
        else if(ui->comboBoxIntrrReqType->currentText() == "Mode C") config.append("2;");
        else if(ui->comboBoxIntrrReqType->currentText() == "Mode S")
        {
            QMessageBox::warning(this,"Input Warning","No Mode S for whisper interrogation.\nPlease select Mode A or C instead",QMessageBox::Ok);
            return;
        }
        else if(ui->comboBoxIntrrReqType->currentText() == "Mode S Extend")
        {
            QMessageBox::warning(this,"Input Warning","No Mode S Extend for whisper interrogation.\nPlease select Mode A or C instead",QMessageBox::Ok);
            return;
        }

        config.append(ui->spinBoxOperationalIntrrFirstAmpl->text()+";");
        config.append(ui->spinBoxOperationalIntrrStepSize->text()+";");
        config.append(ui->spinBoxOperationalIntrrStepCount->text()+";");
        config.append(QString::number(ui->comboBoxOperationalIntrrAntenna->currentIndex())+";");
        config.append(ui->spinBoxOperationalIntrrSuppress->text());

    }
    iff->setInterrogationMode(single_mode,ui->radioButtonOperationalIntrrOnce->isChecked(),config);
}

void DialogIFF::on_comboBoxOperationalIntrrMode_activated(const QString &arg1)
{
    if(arg1 == "Single")
    {
        ui->lineEditOperationalIntrrICAO->setEnabled(true);
        ui->spinBoxOperationalIntrrAmpl->setEnabled(true);
        ui->tabOperationalWhisper->setEnabled(false);
    }
    else
    {
        ui->lineEditOperationalIntrrICAO->setEnabled(false);
        ui->spinBoxOperationalIntrrAmpl->setEnabled(false);
        ui->tabOperationalWhisper->setEnabled(true);
    }
}

void DialogIFF::on_pushButtonBITSendReq_clicked()
{
    iff->dataReq("3"); //status message
}

void DialogIFF::on_pushButtonSettingsInstallation_clicked()
{
    QStringList config = QStringList()<<"ABC123"
                                     <<"TEST01Z"
                                    <<"0"
                                   <<"0"
                                  <<"192.168.1.184"
                                 <<"255.255.255.0"
                                <<"10000"
                               <<"0"
                              <<"0"
                             <<"0"
                            <<"1"
                           <<"3"
                          <<"0"
                         <<"1"
                           ;

    iff->setInstallation(config.join(";")); //tes
}

void DialogIFF::on_pushButtonApplyOperatingUpdateGps_clicked()
{
    iff->setLatLonHdt(currentOwnShipLat, currentOwnShipLon, currentHeading);
}

void DialogIFF::on_pushButtonOpFriendListRemove_clicked()
{
    int row_count = codeFriendListModel->rowCount();

    if(row_count>0)
    {
        int row = ui->tableViewOpFriendList->currentIndex().row();
        QString row_data = ui->tableViewOpFriendList->currentIndex().data().toString();

        codeFriendListModel->removeRow(row);
        friendListCode.remove(row_data);
        friendCodeReplyList.remove(row_data);
        emit signal_friendCodeRemoved(row_data);

        qDebug()<<Q_FUNC_INFO<<"remove code"<<row_data<<row_data;
    }
}

void DialogIFF::on_pushButtonOpFriendListAdd_clicked()
{
    QString code = ui->lineEditFriendListCode->text();
    if(!code.isEmpty())
    {
        if(code.size() == 4)
        {
            for(int i=0; i<code.size(); i++)
            {
                QChar digit =code.at(i);

                if(!digit.isDigit())
                {
                    QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode must contains digit",QMessageBox::Ok);
                    return;
                }
            }

            if(!friendListCode.contains(code))
            {
                friendListCode.insert(code);
                friendCodeReplyList.insert(code,QDateTime::currentSecsSinceEpoch());

                codeFriendListModel->insertRow(codeFriendListModel->rowCount(), QModelIndex());
                codeFriendListModel->setData(codeFriendListModel->index(codeFriendListModel->rowCount()-1,0,QModelIndex()),code);
                codeFriendListModel->item(codeFriendListModel->rowCount()-1,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            }
            else QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode already exist",QMessageBox::Ok);
        }
        else QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode must contains 4 digit",QMessageBox::Ok);
    } else QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode must not empty",QMessageBox::Ok);

}

void DialogIFF::on_pushButtonOPCon1Apply_clicked()
{
    iff_settings.ip1 = ui->lineEditOPIPCon1->text().remove(" ");
    iff_settings.port1 = ui->lineEditOPPortCon1->text().toUInt();

    emit signal_settingChange();
}

void DialogIFF::on_pushButtonOpHostileListRemove_clicked()
{
    int row_count = codeHostileListModel->rowCount();

    if(row_count>0)
    {
        int row = ui->tableViewOpHostileList->currentIndex().row();
        QString row_data = ui->tableViewOpHostileList->currentIndex().data().toString();

        codeHostileListModel->removeRow(row);
        hostileListCode.remove(row_data);
        emit signal_hostileCodeRemoved(row_data);

        qDebug()<<Q_FUNC_INFO<<"remove code"<<row_data<<row_data;
    }

}

void DialogIFF::on_pushButtonOpHostileListAdd_clicked()
{
    QString code = ui->lineEditHostileListCode->text();
    if(!code.isEmpty())
    {
        if(code.size() == 4)
        {
            for(int i=0; i<code.size(); i++)
            {
                QChar digit =code.at(i);

                if(!digit.isDigit())
                {
                    QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode must contains digit",QMessageBox::Ok);
                    return;
                }
            }

            if(!hostileListCode.contains(code))
            {
                hostileListCode.insert(code);

                codeHostileListModel->insertRow(codeHostileListModel->rowCount(), QModelIndex());
                codeHostileListModel->setData(codeHostileListModel->index(codeHostileListModel->rowCount()-1,0,QModelIndex()),code);
                codeHostileListModel->item(codeHostileListModel->rowCount()-1,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

                emit signal_hostileCodeAdded(code);
            }
            else QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode already exist",QMessageBox::Ok);
        }
        else QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode must contains 4 digit",QMessageBox::Ok);
    } else QMessageBox::critical(this,"Input Error","Invalid Code input.\nCode must not empty",QMessageBox::Ok);
}
