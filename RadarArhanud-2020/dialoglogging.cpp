#include "dialoglogging.h"
#include "ui_dialoglogging.h"

#include <log4qt/logger.h>

#include <QFile>
#include <QDir>
#include <QDebug>

DialogLogging::DialogLogging(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogging)
{
    ui->setupUi(this);

    curLogMode = true;
    curFilterColumn = ui->comboBoxFilter->currentIndex()-1;

    QStandardItem *item1 = new QStandardItem("Time");
    QStandardItem *item2 = new QStandardItem("Type");
    QStandardItem *item3 = new QStandardItem("Message");

    logViewModel = new QStandardItemModel(this);
    logViewModel->setColumnCount(3);
    logViewModel->setHorizontalHeaderItem(0,item1);
    logViewModel->setHorizontalHeaderItem(1,item2);
    logViewModel->setHorizontalHeaderItem(2,item3);

    ui->tableViewLog->setModel(logViewModel);
    ui->tableViewLog->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewLog->horizontalHeader()->resizeSection(0,160);
    ui->tableViewLog->horizontalHeader()->resizeSection(1,80);

    curFilename = QDir::homePath()+QDir::separator()+".radar_A3.log";
    ui->labelCurLog->setText(".radar_A3.log");
    openAndReadFile(curFilename);
}

void DialogLogging::insertLog(const QString &msg)
{
    if(curLogMode)
    {
        QString time_section,type_section,msg_section;
        int idx_space,nxt_idx_space;

        time_section = msg.left(19);
        idx_space = msg.indexOf(" ",19);
        nxt_idx_space = msg.indexOf(" ",idx_space+1);
        type_section = msg.mid(idx_space,nxt_idx_space-idx_space);
        idx_space = msg.indexOf(" ",nxt_idx_space);
        msg_section = msg.mid(idx_space,msg.size()-idx_space-1);

        logViewModel->insertRow(logViewModel->rowCount(),QModelIndex());
        logViewModel->setData(logViewModel->index(logViewModel->rowCount()-1,0,QModelIndex()),
                              time_section);
        logViewModel->setData(logViewModel->index(logViewModel->rowCount()-1,1,QModelIndex()),
                              type_section);
        logViewModel->setData(logViewModel->index(logViewModel->rowCount()-1,2,QModelIndex()),
                              msg_section);

        logViewModel->item(logViewModel->rowCount()-1,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);


    }
}

DialogLogging::~DialogLogging()
{
    delete ui;
}

void DialogLogging::on_pushButtonLatestLog_clicked()
{
    curLogMode = true;
    curFilename = QDir::homePath()+QDir::separator()+".radar_A3.log";
    ui->labelCurLog->setText(".radar_A3.log");
    openAndReadFile(curFilename);
}

void DialogLogging::openAndReadFile(const QString &filename)
{
    qApp->setOverrideCursor(Qt::WaitCursor);
    QFile log_file(filename);

    qDebug()<<Q_FUNC_INFO<<filename;
    if(log_file.open(QIODevice::ReadOnly))
    {
        logViewModel->removeRows(0,logViewModel->rowCount()-1);
        log_file.seek(0);

        QString line_read;
        while(!log_file.atEnd())
        {
            line_read = QString(log_file.readLine());
            insertLog(line_read);
        }
    }
    qApp->setOverrideCursor(Qt::ArrowCursor);
}

void DialogLogging::on_lineEditFilter_textChanged(const QString &arg1)
{
    for(int i=0; i<logViewModel->rowCount(); i++)
    {
        QStandardItem *item = logViewModel->item(i,curFilterColumn);
        if(item != nullptr)
        {
            if(!item->text().contains(arg1))
                ui->tableViewLog->hideRow(i);
            else
                ui->tableViewLog->showRow(i);
        }
    }
}

void DialogLogging::on_comboBoxFilter_currentIndexChanged(int index)
{
    curFilterColumn = index-1;
}
