#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QValidator>
#include <QMessageBox>
#include <QHostAddress>
#include <QDebug>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化相关变量
    currClient = nullptr;
    srcFile = nullptr;
    recvFile = nullptr;
    isHead=true;
    //限制输入类型，利用正则表达式
    ui->ip_le->setValidator(new QRegExpValidator(QRegExp("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-4]|2[0-4][0-9]|[01]?[0-9][0-9]?)&"),ui->ip_le));
    ui->port_le->setValidator(new QRegExpValidator(QRegExp("^(?:[0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$"),ui->port_le));
    ui->port_le_server->setValidator(new QRegExpValidator(QRegExp("^(?:[0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$"),ui->port_le_server));
    ui->cancle_pb->setEnabled(false);
    //初始化tcp服务端和客户端
    tcpClient = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);
    //链接信号与槽
    connect(tcpClient,SIGNAL(connected()),this,SLOT(send_Head()));
    connect(this,SIGNAL(waitForConfirm(qint64)),this,SLOT(confirm_Head(qint64)));
    connect(this,SIGNAL(readyForSendData()),this,SLOT(start_send_Data()));
    //connect(tcpClient,SIGNAL(bytesWritten(qint64)),this,SLOT(continue_send_Data(qint64)));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(client_Error()));
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(creat_Connection()));   //连接请求处理
    connect(tcpServer,SIGNAL(acceptError(QAbstractSocket::SocketError)),this,SLOT(server_connection_Error()));

    ui->port_le->setText(config.defaultPort);
    ui->port_le_server->setText(config.defaultPort);
    ui->path_le_server->setText(config.defaultRecvPath);
    blockSize=config.diskCacheSize;
}

MainWindow::~MainWindow()
{
    delete tcpClient;
    delete tcpServer;
    delete ui;
}

void MainWindow::creat_Connection()
{
    currClient=tcpServer->nextPendingConnection();
    //QMessageBox::information(this,"Notcie","A new connection form");
    //tcpCLient_List<<currClient;
    connect(currClient,SIGNAL(readyRead()),this,SLOT(read_Data()));  //读取准备
    connect(currClient,SIGNAL(disconnected()),this,SLOT(cls_currConnection()));  //掉线处理
    connect(currClient,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(server_Error()));

}

void MainWindow::read_Data()
{
    if(isHead)
    {
        QDataStream in(currClient);
        in >> headInfo >> recv_fileName >> recv_fileSize;
        if(QMessageBox::information(this,tr("Server"),"from: "+currClient->peerAddress().toString().mid(7)+"\nFile name: "+recv_fileName+"\nFile size: "+QString::number(recv_fileSize/1000000.0)+"mb",QMessageBox::Yes | QMessageBox::No,QMessageBox::No)==QMessageBox::No)
        {
            currClient->write("##refused##");
            currClient->close();
            return;
        }
        else
        {
            currClient->write("##confirm##");
        }
        recvPath.setPath(ui->path_le_server->text());
        if(!recvPath.exists())
            recvPath.mkpath(recvPath.path());
        recvFile = new QFile(ui->path_le_server->text()+"/"+recv_fileName);
        if(!recvFile->open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this,tr("Server"),tr("Faild to create file"));
            currClient->close();
            delete recvFile;
            isHead=true;
            return;
        }

        remaining_fileSize = recv_fileSize;
        ui->process_server->setValue(0);
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
        ui->process_server->setValue(0);
        recvFile->close();
        currClient->close();
        delete recvFile;
        isHead=true;
        QMessageBox::information(this,tr("Server"),("Finished"));
    }
}

void MainWindow::cls_currConnection()
{
    if(recvFile==nullptr)
        return;
    if(!recvFile->isOpen())
        return;
    QMessageBox::information(this,tr("Server"),tr("Connection have been closed."));
    currClient->close();
    recvFile->close();
    ui->process_server->setValue(0);
}

void MainWindow::client_Error()
{
    QMessageBox::warning(this,tr("Client"),tcpClient->errorString());
    if(tcpClient->isOpen())
        tcpClient->close();
    ui->send_pb->setEnabled(true);
}

void MainWindow::server_Error()
{
    QMessageBox::warning(this,tr("Server"),currClient->errorString());
    if(currClient->isOpen())
        currClient->close();
}

void MainWindow::server_connection_Error()
{
    QMessageBox::warning(this,tr("Server"),tcpServer->errorString());
}

void MainWindow::send_Head()
{
    //关闭发送按钮
    ui->send_pb->setEnabled(false);
    QDataStream out(&src_fileCache,QIODevice::WriteOnly);//发送文件流
    //准备发送的文件
    srcFile = new QFile(srcFileInfo.absoluteFilePath());
    if(!srcFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,tr("Client"),tr("Faild to open file"));
        tcpClient->close(); //关闭tcp客户端
        delete srcFile; //delete防止内存泄漏
        return;
    };
    //取得待发送文件大小
    tosend_fileSize = src_fileSize = srcFile->size();
    //设置输出流文件版本，初始化文件头（文件头，文件大小）
    out.setVersion(QDataStream::Qt_5_13);
    out<<QString("##head##")<<srcFileInfo.fileName()<<src_fileSize;
    //发送文件头数据
    tcpClient->write(src_fileCache);
    emit waitForConfirm(src_fileCache.size());
}

void MainWindow::confirm_Head(qint64 headSize)
{
    if(tcpClient->waitForReadyRead(10000))
    {
        QApplication::processEvents();
        if(QString(tcpClient->readAll())=="##refused##")
        {
            QMessageBox::warning(this,tr("Client"),tr("Sever refues receive file"));
            srcFile->close();
            delete srcFile;
            tcpClient->close();
            ui->send_pb->setEnabled(true);
            return;
        }
    }
    ui->process->setValue(0);
    //初始化已发送文件大小，减去文件头大小
    sended_fileSize = -headSize;
    src_fileCache.resize(0);
    emit  readyForSendData();
}

void MainWindow::start_send_Data()
{
    //已发送文件大小
    qint64 sizeOfSend = 0;
    while(tosend_fileSize>0)
    {
        src_fileCache=srcFile->read(qMin(tosend_fileSize,blockSize));        //将数据块写入缓存
        sizeOfSend = tcpClient->write(src_fileCache);
        tosend_fileSize -= sizeOfSend;
        sended_fileSize += sizeOfSend;
        src_fileCache.resize(0);
        ui->process->setValue(int(double(sended_fileSize)*100/double(src_fileSize)));   //设置进度条
        tcpClient->waitForBytesWritten();
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
        ui->process->setValue(0);//重置进度条
        srcFile->close();   //关闭源文件
        tcpClient->close(); //关闭tcp客户端
        delete srcFile; //delete防止内存泄漏
        QMessageBox::information(this,tr("Client"),tr("Successful!"));
        ui->send_pb->setEnabled(true);
}

void MainWindow::on_listen_pb_clicked()
{
    if(ui->port_le_server->text().toInt()<1025)
    {
        QMessageBox::warning(this,tr("Server"),tr("Faild to listen port"));
        return;
    }
    if(!tcpServer->listen(QHostAddress::Any,ui->port_le_server->text().toUShort())) //开始监听
    {
        QMessageBox::warning(this,tr("Server"),tr("Faild to listen port"));
        return;
    }
    ui->listen_pb->setEnabled(false);   //设置listen按钮不可用
    ui->cancle_pb->setEnabled(true);    //设置cancle按钮可用

}

void MainWindow::on_cancle_pb_clicked()
{
    ui->listen_pb->setEnabled(true);    //设置listen按钮可用
    ui->cancle_pb->setEnabled(false);   //设置cancle按钮不可用
    tcpServer->close(); //停止监听
}

void MainWindow::on_send_pb_clicked()
{
    if(ui->file_le->text().isEmpty())
    {
        QMessageBox::warning(this,tr("Client"),tr("File path can not be empty!"));
        return;
    }
    tcpClient->connectToHost(ui->ip_le->text(),ui->port_le->text().toUShort());    //连接到服务器
}
//文件以及目录选择
void MainWindow::on_pick_pb_clicked()
{
    //选取待发送文件
    srcFileInfo=QFileDialog::getOpenFileName();
    ui->file_le->setText(srcFileInfo.absoluteFilePath());
}

void MainWindow::on_pick_pb_server_clicked()
{
    //选取接收目录
    ui->path_le_server->setText(QFileDialog::getExistingDirectory());
}
