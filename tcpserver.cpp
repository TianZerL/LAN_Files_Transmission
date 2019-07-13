#include "tcpserver.h"
#include "tcpserverthread.h"
#include <QThread>
#include <QMessageBox>

TcpServer::TcpServer(QObject *parent):
    QTcpServer (parent)
{

}

void TcpServer::confirmForReadData(QString IP, QString fileName, qint64 fileSize)
{
    emit confirmResult(true,path);
//    if(QMessageBox::information(nullptr,tr("Server"),"from: "+IP+"\nFile name: "+fileName+"\nFile size: "+QString::number(fileSize/1000000.0)+"mb",QMessageBox::Yes | QMessageBox::No,QMessageBox::No) == QMessageBox::No)
//        emit confirmResult(false,path);
//    else
//        emit confirmResult(true,path);
}

void TcpServer::progressBarValueForUi(int value)
{
    emit ProgressBarValue(value);
}

void TcpServer::transferError(QAbstractSocket::SocketError errornum)
{
    emit error(errornum);
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    TcpServerThread *thread = new TcpServerThread(socketDescriptor);
    QThread *threadContainer = new QThread(this);
    thread->moveToThread(threadContainer);
    connect(threadContainer,SIGNAL(finished()),thread,SLOT(deleteLater()));
    connect(threadContainer,SIGNAL(finished()),threadContainer,SLOT(deleteLater()));
    connect(thread,SIGNAL(needConfirm(QString,QString,qint64)),this,SLOT(confirmForReadData(QString,QString,qint64)));
    connect(this,SIGNAL(confirmResult(bool,QDir)),thread,SLOT(confirm(bool,QDir)));
    connect(thread,SIGNAL(progress(int)),this,SLOT(progressBarValueForUi(int)));
    connect(thread,SIGNAL(error(QTcpSocket::SocketError)),this,SLOT(transferError(QTcpSocket::SocketError)));
    connect(thread,SIGNAL(readFinished()),threadContainer,SLOT(quit()));
    connect(threadContainer,SIGNAL(started()),thread,SLOT(inil()));
    threadContainer->start();
}

QDir TcpServer::getPath() const
{
    return path;
}

void TcpServer::setPath(const QDir &value)
{
    path = value;
}
