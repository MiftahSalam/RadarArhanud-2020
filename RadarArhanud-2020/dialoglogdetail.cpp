#include "dialoglogdetail.h"
#include "ui_dialoglogdetail.h"

const QString detail_format =
        "<html>"
            "<head/>"
            "<body>"
                "<p><span style=\" font-weight:600; color:#ffff00;\">Message</span></p>"
                "<p>%1</p>"
                "<p><br/></p>"
                "<p><span style=\" font-weight:600; color:#ffff00;\">Remark/Suggestion</span></p>"
                "<p>%2</p>"
            "</body>"
        "</html>";

DialogLogDetail::DialogLogDetail(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogDetail)
{
    ui->setupUi(this);
}

void DialogLogDetail::setMessage(const QString msg)
{
    QString remark = "-";
    QString detail_text;

    if(msg.contains("QNetworkReplyImplPrivate::error: Internal problem"))
        remark = "Cannot connect to map server. Please check internet connection";

    detail_text = QString(detail_format).arg(msg).arg(remark);

    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendHtml(detail_text);

}

void DialogLogDetail::setDate(const QString date)
{
    ui->labelDate->setText(date);
}

void DialogLogDetail::setType(const QString type)
{
    ui->labelType->setText(type);
}

void DialogLogDetail::setSource(const QString source)
{
    ui->labelSource->setText(source);
}

DialogLogDetail::~DialogLogDetail()
{
    delete ui;
}
