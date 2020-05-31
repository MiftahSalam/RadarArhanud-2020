#include "dialogopenloghistory.h"
#include "ui_dialogopenloghistory.h"

DialogOpenLogHistory::DialogOpenLogHistory(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOpenLogHistory)
{
    ui->setupUi(this);

    logListModel = new QStringListModel(this);
    ui->listView->setModel(logListModel);
}

void DialogOpenLogHistory::setFileList(const QStringList &files)
{
    logListModel->setStringList(files);
}

DialogOpenLogHistory::~DialogOpenLogHistory()
{
    delete ui;
}

void DialogOpenLogHistory::on_buttonBox_accepted()
{
    QString selected = ui->listView->currentIndex().data().toString();

    emit signal_openFile(selected);
}

void DialogOpenLogHistory::on_listView_doubleClicked(const QModelIndex &index)
{
    QString selected = index.data().toString();

    emit signal_openFile(selected);
}
