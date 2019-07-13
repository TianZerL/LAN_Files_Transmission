#include "tcpserverthread.h"
#include <QDataStream>
#include <QHostAddress>

TcpServerThread::TcpServerThread(qintptr _socketDescriptor,QObject *parent) : QObject(parent),socketDescriptor(_socketDescriptor)
{
}

TcpServerThread::~TcpServerThread()
{
    if(socket->isOpen())
        socket->close();
    delete recvFile;
    delete socket;
}

void TcpServerThread::inil()
{
    socket = new QTcpSocket;
    if(!socket->setSocketDescriptor(socketDescriptor))
    {
        emit error(socket->error());
        return;
    }
    connect(socket,SIGNAL(readyRead()),this,SLOT(readyConfirm()));
}

void TcpServerThread::readyConfirm()
{
    disconnect(socket,SIGNAL(readyRead()),this,SLOT(confirm()));

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
    }
    else
    {
        recvPath=_recvPath;
        if(!recvPath.exists())
            recvPath.mkpath(recvPath.path());
        recvFile = new QFile(recvPath.path()+"/"+recv_fileName);
        if(!recvFile->open(QIODevice::WriteOnly))
        {
            socket->close();
            delete recvFile;
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
        socket->close();
        emit progress(0);
        emit readFinished();
    }
}
