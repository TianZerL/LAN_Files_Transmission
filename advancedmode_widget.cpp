#include "advancedmode_widget.h"
#include "ui_advancedmode_widget.h"
#include <QNetworkConfigurationManager>
#include <QMessageBox>
#include <QDebug>
#include <QRegExpValidator>

advancedmode_widget::advancedmode_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::advancedmode_widget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    currFile = nullptr;
    cancleFlag = false;

    ui->ipaddress_le->setReadOnly(true);
#ifdef Q_OS_WIN32
    ui->ipaddress_le->setText(QHostInfo::fromName(QHostInfo::localHostName()).addresses().at(1).toString());
#else
    ui->ipaddress_le->setText(QHostInfo::fromName(QHostInfo::localHostName()).addresses().at(0).toString());
#endif
    ui->hostname_le->setReadOnly(true);
    ui->hostname_le->setText(QHostInfo::localHostName());
    //限制输入类型，利用正则表达式
    ui->ip_le->setValidator(new QRegExpValidator(QRegExp("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-4]|2[0-4][0-9]|[01]?[0-9][0-9]?)&"),ui->ip_le));
    ui->port_le->setValidator(new QRegExpValidator(QRegExp("^(?:[0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$"),ui->port_le));
    ui->port_le_server->setValidator(new QRegExpValidator(QRegExp("^(?:[0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$"),ui->port_le_server));
    ui->cancle_pb->setEnabled(false);
    ui->cancle_pb_client->setEnabled(false);

    tcpClient = new QTcpSocket(this);
    tcpServer = new TcpServer(this, TcpServer::MultiThread);

    ui->port_le->setText(config.defaultPort);
    ui->port_le_server->setText(config.defaultPort);
    ui->path_le_server->setText(config.defaultRecvPath);
    blockSize=config.diskCacheSize;

    connect(tcpClient,SIGNAL(connected()),this,SLOT(send_Head()));
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(confirm_Head()));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(client_Error()));
    connect(tcpServer,SIGNAL(error(QString)),this,SLOT(server_Error(QString)));
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(creat_Connection()));   //连接请求处理
    connect(tcpServer,SIGNAL(acceptError(QAbstractSocket::SocketError)),this,SLOT(server_connection_Error()));

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
    disconnect(tcpClient,SIGNAL(readyRead()),this,SLOT(confirm_Head()));
    cancleFlag = false;
    qint64 sizeOfSend = 0;
    sended_fileSize = 0;
    for (i = 0; i < srcFileList.size(); i++)
    {
        QString _fileName=srcFileList.at(i).fileName(),_filePath=srcFileList.at(i).canonicalPath().mid(srcPath.path().length()+1);
        qint64 _fileSize = srcFileList.at(i).size();

        QDataStream out(&src_fileCache,QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_13);
        out << _fileName << _filePath << _fileSize;
        tcpClient->write(src_fileCache);

        src_fileCache.resize(0);
        currFile = new QFile(srcFileList.at(i).filePath());
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
        ui->cancle_pb_client->setEnabled(true);
        tcpClient->waitForReadyRead();
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
        tcpClient->waitForReadyRead();
    }
    ui->process->reset();
    tcpClient->close();
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(confirm_Head()));
    if(!cancleFlag)
        QMessageBox::information(this,tr("Client"),tr("Successful!"));
    else
        QMessageBox::information(this,tr("Client"),tr("cancled!"));
    ui->send_pb->setEnabled(true);
    ui->cancle_pb_client->setEnabled(false);
}

void advancedmode_widget::client_Error()
{
    tosend_fileSize = 0;
    i = srcFileList.size();
    cancleFlag = true;
    QMessageBox::warning(this,tr("Client"),tcpClient->errorString());
    if(tcpClient->isOpen())
        tcpClient->close();
    ui->send_pb->setEnabled(true);
    ui->process->reset();
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

void advancedmode_widget::on_cancle_pb_client_clicked()
{
    if(QMessageBox::warning(this,"Waring","Are you sure to cancle?",QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        return;
    tcpClient->disconnectFromHost();
    tosend_fileSize = 0;
    i = srcFileList.size();
}

void advancedmode_widget::on_listen_pb_clicked()
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

void advancedmode_widget::on_pick_pb_server_clicked()
{
    ui->path_le->setText(QFileDialog::getExistingDirectory());
}

void advancedmode_widget::creat_Connection()
{
    tcpServer->setPath(ui->path_le_server->text());
}

void advancedmode_widget::server_Error(QString errorString)
{
    QMessageBox::warning(this,tr("Server"),errorString);
}

void advancedmode_widget::server_connection_Error()
{
    QMessageBox::warning(this,tr("Server"),tcpServer->errorString());
}
