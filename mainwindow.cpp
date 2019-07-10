#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QValidator>
#include <QMessageBox>
#include <QHostAddress>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    currClient = nullptr;
    file_src = nullptr;
    file_recv = nullptr;
    block_size = 4096;
    head_flag=true;

    ui->ip1_le->setValidator(new QIntValidator(0,255,ui->ip1_le));
    ui->ip2_le->setValidator(new QIntValidator(0,255,ui->ip2_le));
    ui->ip3_le->setValidator(new QIntValidator(0,255,ui->ip3_le));
    ui->ip4_le->setValidator(new QIntValidator(0,254,ui->ip4_le));
    ui->port_le->setValidator(new QIntValidator(0,65535,ui->port_le));
    ui->port_le_server->setValidator(new QIntValidator(0,65535,ui->port_le_server));
    ui->cancle_pb->setEnabled(false);

    tcpClient = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);

    connect(tcpClient,SIGNAL(connected()),this,SLOT(start_send_Data()));
    connect(tcpClient,SIGNAL(bytesWritten(qint64)),this,SLOT(continue_send_Data(qint64)));
}

MainWindow::~MainWindow()
{
    delete file_src;
    delete file_recv;
    delete in;
    delete out;
    delete tcpClient;
    delete tcpServer;
    delete ui;
}

void MainWindow::creat_Connection()
{
    currClient=tcpServer->nextPendingConnection();
    in = new QDataStream(currClient);
    //tcpCLient_List<<currClient;
    connect(currClient,SIGNAL(readyRead()),this,SLOT(read_Data()));  //读取准备
    connect(currClient,SIGNAL(disconnected()),this,SLOT(cls_Connection()));  //掉线处理
}

void MainWindow::read_Data()
{
    if(head_flag)
    {
        *in>>head>>file_name>>file_size_sended;
        delete file_recv;
        file_recv = new QFile(file_name+"_recv");
        file_size_recv=file_size_sended;
        file_recv->open(QIODevice::WriteOnly);
        head_flag=false;
    }
    if(file_size_recv >0 && !head_flag )
    {
        file_recv_cache=currClient->readAll();
        file_size_recv-=file_recv->write(file_recv_cache);
    }
    if(file_size_recv==0)
    {
        QMessageBox::information(this,"1","1");
        file_recv->close();
        currClient->close();
    }
}

void MainWindow::cls_Connection()
{

}

void MainWindow::start_send_Data()
{
    out = new QDataStream(&file_src_cache,QIODevice::WriteOnly);//发送文件流

    delete file_src;
    file_src = new QFile("test_path_src");

    file_src->open(QIODevice::ReadOnly);
    file_size_src_to_send = file_size_src = file_src->size();
    out->setVersion(QDataStream::Qt_5_13);

    *out<<QString("##head##")<<file_src->fileName()<<file_size_src;//<<file_src->read(qMin(file_size_src,block_size));
    tcpClient->write(file_src_cache);
    file_src_cache.resize(0);
}

void MainWindow::continue_send_Data(qint64 size_of_bytes)
{
    file_size_src_sended += size_of_bytes;
    if(file_size_src_to_send>0)
    {
        file_src_cache=file_src->read(qMin(file_size_src,block_size));
        file_size_src_to_send -= size_of_bytes;
        tcpClient->write(file_src_cache);
        file_src_cache.resize(0);
    }
    else
    {
        file_src->close();
        tcpClient->close();
    }

}

void MainWindow::on_listen_pb_clicked()
{
    ui->listen_pb->setEnabled(false);
    ui->cancle_pb->setEnabled(true);
    tcpServer->listen(QHostAddress::Any,ui->port_le_server->text().toUShort()); //开始监听
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(creat_Connection()));   //连接请求处理
}

void MainWindow::on_cancle_pb_clicked()
{
    ui->listen_pb->setEnabled(true);
    ui->cancle_pb->setEnabled(false);
    tcpServer->close(); //停止监听
}

void MainWindow::on_send_pb_clicked()
{
    tcpClient->connectToHost(ui->ip1_le->text()+"."+ui->ip2_le->text()+"."+ui->ip3_le->text()+"."+ui->ip4_le->text(),ui->port_le->text().toUShort());
}
