#include "tcpserverthread.h"
#include <QDataStream>
#include <QHostAddress>

TcpServerThread::TcpServerThread(qintptr _socketDescriptor,QObject *parent) : QObject(parent),socketDescriptor(_socketDescriptor)
{
    socket=nullptr;
    recvFile=nullptr;
}

TcpServerThread::~TcpServerThread()
{
    if(recvFile!=nullptr)
    {
        if(recvFile->isOpen())
        {
            recvFile->close();
            delete recvFile;
            recvFile=nullptr;
        }
        else
        {
            delete recvFile;
            recvFile=nullptr;
        }
    }
    if(socket->isOpen())
        socket->close();
    delete socket;
    socket=nullptr;
}

void TcpServerThread::inil()
{
    socket = new QTcpSocket;
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(getError()));
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket,SIGNAL(readyRead()),this,SLOT(readyConfirm()));
}

void TcpServerThread::readyConfirm()
{
    disconnect(socket,SIGNAL(readyRead()),this,SLOT(readyConfirm()));

    QDataStream in(socket);
    in >> headInfo >> recv_fileName >> recv_fileSize;
    emit needConfirm(socket->peerAddress().toString().mid(7),recv_fileName,recv_fileSize);
}

void TcpServerThread::confirm(bool signal,QDir _recvPath)
{
    if(!signal)
    {
        socket->write("##refused##");
        socket->close();
        emit refuseConnection();
    }
    else
    {
        recvPath=_recvPath;
        if(!recvPath.exists())
            recvPath.mkpath(recvPath.path());
        recvFile = new QFile(recvPath.path()+"/"+recv_fileName);
        if(recvFile->exists())
            recvFile->setFileName(recvPath.path()+"/recv_"+recv_fileName);
        if(!recvFile->open(QIODevice::WriteOnly))
        {
            socket->close();
            delete recvFile;
            recvFile=nullptr;
            return;
        }
        remaining_fileSize = recv_fileSize;
        connect(socket,SIGNAL(readyRead()),this,SLOT(readData()));
        socket->write("##confirm##");
    }
}

void TcpServerThread::readData()
{
    if(remaining_fileSize >0)
    {
        recv_fileCache=socket->readAll();
        remaining_fileSize-=recvFile->write(recv_fileCache);
        recvFile->flush();
        emit progress(int((1.0-(double(remaining_fileSize)/double(recv_fileSize)))*100));
    }
    if(remaining_fileSize <= 0)
    {
        recvFile->close();
        delete recvFile;
        recvFile=nullptr;
        socket->close();
        emit progress(0);
        emit readFinished();
    }
}

void TcpServerThread::getError()
{
    emit error(socket->errorString());
}
