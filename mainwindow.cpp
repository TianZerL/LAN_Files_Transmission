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
    //初始化相关变量
    currClient = nullptr;
    srcFile = nullptr;
    recvFile = nullptr;

    blockSize = 4096;
    isHead=true;
    //限制输入类型
    ui->ip1_le->setValidator(new QIntValidator(0,255,ui->ip1_le));
    ui->ip2_le->setValidator(new QIntValidator(0,255,ui->ip2_le));
    ui->ip3_le->setValidator(new QIntValidator(0,255,ui->ip3_le));
    ui->ip4_le->setValidator(new QIntValidator(0,254,ui->ip4_le));
    ui->port_le->setValidator(new QIntValidator(0,65535,ui->port_le));
    ui->port_le_server->setValidator(new QIntValidator(0,65535,ui->port_le_server));
    ui->cancle_pb->setEnabled(false);
    //初始化tcp服务端和客户端
    tcpClient = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);
    //链接信号与槽
    connect(tcpClient,SIGNAL(connected()),this,SLOT(start_send_Data()));
    connect(tcpClient,SIGNAL(bytesWritten(qint64)),this,SLOT(continue_send_Data(qint64)));
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(creat_Connection()));   //连接请求处理

}

MainWindow::~MainWindow()
{
    delete srcFile;
    delete recvFile;
    delete tcpClient;
    delete tcpServer;
    delete ui;
}

void MainWindow::creat_Connection()
{
    currClient=tcpServer->nextPendingConnection();
    //tcpCLient_List<<currClient;
    connect(currClient,SIGNAL(readyRead()),this,SLOT(read_Data()));  //读取准备
    connect(currClient,SIGNAL(disconnected()),this,SLOT(cls_Connection()));  //掉线处理

}

void MainWindow::read_Data()
{
    if(isHead)
    {
        QDataStream in(currClient);
        in >> headInfo >> recv_fileName >> recv_fileSize;

        delete recvFile;
        recvFile = new QFile(recv_fileName+"_recv");

        remaining_fileSize = recv_fileSize;
        recvFile->open(QIODevice::WriteOnly);
        isHead=false;
    }
    if(remaining_fileSize >0 && !isHead )
    {
        recv_fileCache=currClient->readAll();
        remaining_fileSize-=recvFile->write(recv_fileCache);
        recvFile->flush();
        ui->process_server->setValue(int((1.0-(double(remaining_fileSize)/double(recv_fileSize)))*100));
    }
    if(remaining_fileSize <= 0)
    {
        QMessageBox::information(this,"Notice","Finished");
        recvFile->close();
        currClient->close();
    }
}

void MainWindow::cls_Connection()
{

}

void MainWindow::start_send_Data()
{
    QDataStream out(&src_fileCache,QIODevice::WriteOnly);//发送文件流
    //准备发送的文件
    delete srcFile; //delete一次防止内存泄漏
    srcFile = new QFile("test_path_src");
    srcFile->open(QIODevice::ReadOnly);
    //取得待发送文件大小
    tosend_fileSize = src_fileSize = srcFile->size();
    //设置输出流文件版本，初始化文件头（文件头，文件大小）
    out.setVersion(QDataStream::Qt_5_13);
    out<<QString("##head##")<<srcFile->fileName()<<src_fileSize;
    //发送文件头数据
    tcpClient->write(src_fileCache);
    ui->process->setValue(0);
    src_fileCache.resize(0);
}

void MainWindow::continue_send_Data(qint64 size_of_bytes)
{
    sended_fileSize += size_of_bytes;
    if(tosend_fileSize>0)
    {
        src_fileCache=srcFile->read(qMin(src_fileSize,blockSize));
        tosend_fileSize -= size_of_bytes;
        tcpClient->write(src_fileCache);
        src_fileCache.resize(0);
        ui->process->setValue(int(double(sended_fileSize)*100/double(src_fileSize)));
    }

    if(tosend_fileSize<=0)
    {
        QMessageBox::information(this,"Notice","Successful!");
        ui->process->setValue(0);
        srcFile->close();   //关闭源文件
        tcpClient->close(); //关闭tcp客户端
    }

}

void MainWindow::on_listen_pb_clicked()
{
    ui->listen_pb->setEnabled(false);
    ui->cancle_pb->setEnabled(true);
    tcpServer->listen(QHostAddress::Any,ui->port_le_server->text().toUShort()); //开始监听

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
