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

    QFile log_file(QDir::homePath()+QDir::separator()+".radar_A3.log");

    if(log_file.open(QIODevice::ReadOnly))
    {
        qDebug()<<Q_FUNC_INFO<<"file open";

        log_file.seek(0);

        QString line_read,time_section,type_section,msg_section;
        int idx_space,nxt_idx_space;
        while(!log_file.atEnd())
        {
            line_read = QString(log_file.readLine());
            time_section = line_read.left(19);
            idx_space = line_read.indexOf(" ",19);
            nxt_idx_space = line_read.indexOf(" ",idx_space+1);
            type_section = line_read.mid(idx_space,nxt_idx_space-idx_space);
            idx_space = line_read.indexOf(" ",nxt_idx_space);
            msg_section = line_read.mid(idx_space,line_read.size()-idx_space-1);

            logViewModel->insertRow(logViewModel->rowCount(),QModelIndex());
            logViewModel->setData(logViewModel->index(logViewModel->rowCount()-1,0,QModelIndex()),
                           time_section);
            logViewModel->setData(logViewModel->index(logViewModel->rowCount()-1,1,QModelIndex()),
                           type_section);
            logViewModel->setData(logViewModel->index(logViewModel->rowCount()-1,2,QModelIndex()),
                           msg_section);

//            Log4Qt::Logger::rootLogger()->trace()<<Q_FUNC_INFO<<log_file.readLine();
        }
    }
}

DialogLogging::~DialogLogging()
{
    delete ui;
}
