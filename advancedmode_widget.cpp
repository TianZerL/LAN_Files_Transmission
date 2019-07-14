#include "advancedmode_widget.h"
#include "ui_advancedmode_widget.h"
#include <QNetworkConfigurationManager>
#include <QMessageBox>
#include <QDebug>

advancedmode_widget::advancedmode_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::advancedmode_widget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    currFile = nullptr;

    ui->ipaddress_le->setReadOnly(true);
    ui->ipaddress_le->setText(QHostInfo::fromName(QHostInfo::localHostName()).addresses()[1].toString());
    ui->hostname_le->setReadOnly(true);
    ui->hostname_le->setText(QHostInfo::localHostName());

    tcpClient = new QTcpSocket(this);
    tcpServer = new TcpServer(this);

    ui->port_le->setText(config.defaultPort);
    ui->port_le_server->setText(config.defaultPort);
    ui->path_le_server->setText(config.defaultRecvPath);
    blockSize=config.diskCacheSize;

    connect(tcpClient,SIGNAL(connected()),this,SLOT(send_Head()));
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(confirm_Head()));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(client_Error()));

    ipCompleter = new IP_Completer(ui->ip_le);
    connect(this,SIGNAL(addIPToList(QString)),ipCompleter,SLOT(addIP(QString)));
    ui->ip_le->setCompleter(ipCompleter->getCompleter());
}

advancedmode_widget::~advancedmode_widget()
{
    delete ipCompleter;
    delete tcpClient;
    delete tcpServer;
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

void advancedmode_widget::send_Head()
{
    //关闭发送按钮
    ui->send_pb->setEnabled(false);
    QDataStream out(&src_fileCache,QIODevice::WriteOnly);//发送文件流
    //准备发送的文件
    src_pathSize = tosend_pathSize = 0;
    for (QFileInfo srcFile : srcFileList)
        src_pathSize+=srcFile.size();
    //取得待发送文件大小
    tosend_pathSize = src_pathSize;
    //设置输出流文件版本，初始化文件头（文件头，文件大小）
    out.setVersion(QDataStream::Qt_5_13);
    qDebug()<<srcFileList.length();
    out<<QString("##dir##")<<srcPath.dirName()<<src_pathSize<<srcFileList.length();
    //发送文件头数据
    tcpClient->write(src_fileCache);
    src_fileCache.resize(0);
}

void advancedmode_widget::confirm_Head()
{
    if((tcpClient->readAll()=="##refused##"))
    {
        QMessageBox::warning(this,tr("Client"),tr("Sever refues receive file"));
        tcpClient->close();
        ui->send_pb->setEnabled(true);
        return;
    }
    start_send_Data();
}

void advancedmode_widget::start_send_Data()
{
    qint64 sizeOfSend = 0;
    sended_fileSize = 0;
    for (QFileInfo srcFile : srcFileList)
    {
        QString _fileName=srcFile.fileName(),_filePath=srcFile.canonicalPath();
        qint64 _fileSize = srcFile.size();
        QDataStream out(&src_fileCache,QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_13);
        out << _fileName << _filePath << _fileSize;
        tcpClient->write(src_fileCache);
        src_fileCache.resize(0);
        disconnect(tcpClient,SIGNAL(readyRead()),this,SLOT(confirm_Head()));
        currFile = new QFile(srcFile.filePath());
        if(!currFile->open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this,tr("Client"),tr("Faild to open file"));
            tcpClient->close(); //关闭tcp客户端
            delete currFile; //delete防止内存泄漏
            currFile=nullptr;
            break;
        }
        sizeOfSend = 0;
        tosend_fileSize = currFile->size();
        ui->process->reset();
        ui->cancle_pb_client->setEnabled(true);
        tcpClient->waitForReadyRead();
        QApplication::processEvents();
        tcpClient->readAll();
        while(tosend_fileSize > 0)
        {
            src_fileCache=currFile->read(qMin(tosend_fileSize,blockSize));        //将数据块写入缓存
            sizeOfSend = tcpClient->write(src_fileCache);
            tosend_fileSize -= sizeOfSend;
            sended_fileSize += sizeOfSend;
            src_fileCache.resize(0);
            ui->process->setValue(int(double(sended_fileSize)*100/double(src_pathSize)));   //设置进度条
            tcpClient->waitForBytesWritten();
            QApplication::processEvents();
        }
        currFile->close();   //关闭源文件
        delete currFile; //delete防止内存泄漏
        currFile=nullptr;
    }
    ui->process->reset();
    tcpClient->close();
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(confirm_Head()));
    QMessageBox::information(this,tr("Client"),tr("Successful!"));
    ui->send_pb->setEnabled(true);
    ui->cancle_pb_client->setEnabled(false);
//    qint64 sizeOfSend = 0;
//    sended_fileSize = 0;
//    ui->process->reset();
//    ui->cancle_pb_client->setEnabled(true);
//    while(tosend_fileSize>0)
//    {
//        src_fileCache=srcFile->read(qMin(tosend_fileSize,blockSize));        //将数据块写入缓存
//        sizeOfSend = tcpClient->write(src_fileCache);
//        tosend_fileSize -= sizeOfSend;
//        sended_fileSize += sizeOfSend;
//        src_fileCache.resize(0);
//        ui->process->setValue(int(double(sended_fileSize)*100/double(src_fileSize)));   //设置进度条
//        tcpClient->waitForBytesWritten();
//        QApplication::processEvents();
//    }
//    ui->process->reset();//重置进度条
//    srcFile->close();   //关闭源文件
//    delete srcFile; //delete防止内存泄漏
//    srcFile=nullptr;
//    tcpClient->close(); //关闭tcp客户端
//    QMessageBox::information(this,tr("Client"),tr("Successful!"));
//    ui->send_pb->setEnabled(true);
//    ui->cancle_pb_client->setEnabled(false);
}

void advancedmode_widget::client_Error()
{
    QMessageBox::warning(this,tr("Client"),tcpClient->errorString());
    if(tcpClient->isOpen())
        tcpClient->close();
    ui->send_pb->setEnabled(true);
}

void advancedmode_widget::getFileList(const QString &path)
{
    QFileInfo srcFileInfo(path);
    if(srcFileInfo.isDir())
    {
        QDir srcDir(path);
        QFileInfoList srcList=srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::DirsFirst);
        if(srcList.isEmpty())
            return;
        for (QFileInfo srcfile : srcList)
            getFileList(srcfile.filePath());
    }
    else
        srcFileList<<srcFileInfo;
}

void advancedmode_widget::on_send_pb_clicked()
{
    if(ui->ip_le->text().count('.')<3||ui->ip_le->text().endsWith('.'))
    {
        QMessageBox::warning(this,tr("Client"),tr("IP is not correct!"));
        return;
    }
    emit addIPToList(ui->ip_le->text());
    if(ui->path_le->text().isEmpty())
    {
        QMessageBox::warning(this,tr("Client"),tr("Dir path can not be empty!"));
        return;
    }
    srcPath.setPath(ui->path_le->text());
    if(!srcPath.exists())
    {
        QMessageBox::warning(this,tr("Client"),tr("Dir path did not exist!"));
        return;
    }
    srcFileList.clear();
    getFileList(srcPath.path());
    tcpClient->connectToHost(ui->ip_le->text(),ui->port_le->text().toUShort());    //连接到服务器
}

void advancedmode_widget::on_pick_pb_clicked()
{
    ui->path_le->setText(QFileDialog::getExistingDirectory());
}
