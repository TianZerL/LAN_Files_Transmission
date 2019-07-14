#include "tcpserver.h"
#include "tcpserverthread.h"
#include <QThread>
#include <QMessageBox>

TcpServer::TcpServer(QObject *parent, ServerMode _serverMode, PermissionMode _permissionMode):
    QTcpServer (parent), serverMode(_serverMode), permissionMode(_permissionMode)
{
    qRegisterMetaType<QDir>("QDir");
    threadFlag = false;
}

void TcpServer::confirmForReadData(QString IP, QString fileName, qint64 fileSize)
{
    if(permissionMode == NeedConfirmation && QMessageBox::information(nullptr,tr("Server"),"from: "+IP+"\nFile name: "+fileName+"\nFile size: "+QString::number(fileSize/1000000.0)+"mb",QMessageBox::Yes | QMessageBox::No,QMessageBox::No) == QMessageBox::No)
        emit confirmResult(false,path);
    else
        emit confirmResult(true,path);
}

void TcpServer::progressBarValueForUi(int value)
{
    emit ProgressBarValue(value);
}

void TcpServer::transferError(QString errorString)
{
    emit error(errorString);
}

void TcpServer::finished()
{
    QMessageBox::information(nullptr,tr("Server"),tr("Receiving file finished!\nPath:")+path.path());
}

void TcpServer::resetThreadFlag()
{
    threadFlag = false;
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    if(threadFlag)
    {
        nextPendingConnection()->close();
        return;
    }
    TcpServerThread *thread = new TcpServerThread(socketDescriptor);    //新线程
    QThread *threadContainer = new QThread(this);   //线程容器
    thread->moveToThread(threadContainer);  //装入容器
    connect(threadContainer,SIGNAL(finished()),thread,SLOT(deleteLater())); //读取结束后自我销毁
    connect(threadContainer,SIGNAL(finished()),threadContainer,SLOT(deleteLater()));    //读取结束后自我销毁    
    connect(thread,SIGNAL(needConfirm(QString,QString,qint64)),this,SLOT(confirmForReadData(QString,QString,qint64)));  //确认请求
    connect(this,SIGNAL(confirmResult(bool,QDir)),thread,SLOT(confirm(bool,QDir))); //确认请求结果
    connect(thread,SIGNAL(progress(int)),this,SLOT(progressBarValueForUi(int)));    //更新进度条
    connect(thread,SIGNAL(error(QString)),this,SLOT(transferError(QString)));   //错误信息
    connect(thread,SIGNAL(error(QString)),threadContainer,SLOT(quit()));    //确保推出线程
    connect(thread,SIGNAL(refuseConnection()),threadContainer,SLOT(quit()));    //确保推出线程
    connect(thread,SIGNAL(readFinished()),threadContainer,SLOT(quit()));    //确保推出线程
    connect(thread,SIGNAL(readFinished()),this,SLOT(finished()));   //读取完成后进行提示
    connect(threadContainer,SIGNAL(started()),thread,SLOT(inil())); //调用新线程
    threadContainer->start();   //开启新线程
    emit newConnection();   //发送新连接信号
    if(serverMode == SingleThread)
    {
        threadFlag = true;
        connect(threadContainer,SIGNAL(finished()),this,SLOT(resetThreadFlag())); //重置标记物
    }
}

QDir TcpServer::getPath() const
{
    return path;
}

void TcpServer::setPath(const QDir &value)
{
    path = value;
}
