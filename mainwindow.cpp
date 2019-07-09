#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QValidator>
#include <QMessageBox>
#include <QHostAddress>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->ip1_le->setValidator(new QIntValidator(0,255,ui->ip1_le));
    ui->ip2_le->setValidator(new QIntValidator(0,255,ui->ip2_le));
    ui->ip3_le->setValidator(new QIntValidator(0,255,ui->ip3_le));
    ui->ip4_le->setValidator(new QIntValidator(0,255,ui->ip4_le));
    ui->port_le->setValidator(new QIntValidator(0,65535,ui->port_le));
    ui->port_le_server->setValidator(new QIntValidator(0,65535,ui->port_le_server));
    tcpClient = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(creat_Connection()));
}

MainWindow::~MainWindow()
{
    delete tcpClient;
    delete tcpServer;
    delete tcpConnection;
    delete ui;
}
