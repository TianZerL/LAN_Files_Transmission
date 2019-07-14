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
    if(headInfo == "##dir##")
    {
        in >> fileCount;
        qDebug()<<fileCount;
        recv_pathName = recv_fileName;
        recv_pathSize = recv_fileSize;
    }
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
        if(headInfo == "##dir##")
        {
            recvPath=QDir(_recvPath.path()+recv_pathName);
            if(!recvPath.exists())
                recvPath.mkpath(recvPath.path());
            remaining_pathSize = recv_pathSize;
            connect(socket,SIGNAL(readyRead()),this,SLOT(readData()));
            socket->write("##confirm##");
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
}

void TcpServerThread::readData()
{
    if(headInfo == "##dir##")
    {
        disconnect(socket,SIGNAL(readyRead()),this,SLOT(readData()));

        for (int i = 0 ; i<fileCount ; i++)
        {
            qint64 tempSize;
            QString _fileName,_filePath;
            qint64 _fileSize;
            socket->waitForReadyRead();
            recv_fileCache=socket->readAll();
            QDataStream in(recv_fileCache);
            in >> _fileName >> _filePath >> _fileSize;
            QDir currPath(recvPath.path()+_filePath);
            recv_fileSize = remaining_fileSize = _fileSize;
            if(!currPath.exists())
                currPath.mkpath(currPath.path());
            recvFile = new QFile(currPath.path()+_fileName);
            if(recvFile->exists())
                recvFile->setFileName(recvPath.path()+"/recv_"+recv_fileName);
            if(!recvFile->open(QIODevice::WriteOnly))
            {
                socket->close();
                delete recvFile;
                recvFile=nullptr;
                break;
            }
            recv_fileCache.resize(0);
            socket->write("##fine##");
            socket->waitForReadyRead();
            while(remaining_fileSize >0)
            {
                recv_fileCache=socket->readAll();
                tempSize=recvFile->write(recv_fileCache);
                remaining_fileSize-= tempSize;
                remaining_pathSize-= tempSize;
                recvFile->flush();
                emit progress(int((1.0-(double(remaining_pathSize)/double(recv_pathSize)))*100));
                recvFile->waitForBytesWritten(-1);
            }
            recvFile->close();
            delete recvFile;
            recvFile=nullptr;
        }
        socket->close();
        emit progress(0);
        emit readFinished();
    }
    else
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
}

void TcpServerThread::getError()
{
    emit error(socket->errorString());
}
