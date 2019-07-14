#include "advancedmode_widget.h"
#include "ui_advancedmode_widget.h"
#include <QNetworkConfigurationManager>
#include <QMessageBox>

advancedmode_widget::advancedmode_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::advancedmode_widget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->ipaddress_le->setReadOnly(true);
    ui->ipaddress_le->setText(QHostInfo::fromName(QHostInfo::localHostName()).addresses()[1].toString());
    ui->hostname_le->setReadOnly(true);
    ui->hostname_le->setText(QHostInfo::localHostName());
}

advancedmode_widget::~advancedmode_widget()
{
    delete ui;
}

void advancedmode_widget::on_lantest_pb_clicked()
{
    if(QNetworkConfigurationManager().isOnline())
        QMessageBox::information(this,"LAN test","LAN is online");
    else
        QMessageBox::warning(this,"LAN test","LAN is offline");
}

void advancedmode_widget::on_wantest_pb_clicked()
{
    QHostInfo::lookupHost("www.baidu.com", this, SLOT(lookedUp(QHostInfo)));
}

void advancedmode_widget::lookedUp(QHostInfo hostInfo)
{
    if(hostInfo.error() == QHostInfo::NoError)
        QMessageBox::information(this,"LAN test","WAN is online");
    else
        QMessageBox::warning(this,"LAN test","WAN is offline");
}
