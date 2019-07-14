#ifndef TCPSERVERTHREAD_H
#define TCPSERVERTHREAD_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTcpSocket>
#include "config.h"

class TcpServerThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerThread(qintptr _socketDescriptor,QObject *parent = nullptr);
    ~TcpServerThread();

signals:
    void error(QString);
    void readFinished();
    void refuseConnection();
    void needConfirm(QString,QString,qint64);
    void progress(int);
public slots:
    void inil();
    void readyConfirm();
    void confirm(bool signal,QDir _recvPath);
    void readData();

    void getError();

private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
    QFile *recvFile;
    QDir recvPath;
    QByteArray recv_fileCache;
    QString headInfo,recv_fileName,recv_pathName;
    qint64 recv_fileSize, remaining_fileSize,recv_pathSize, remaining_pathSize;
    int  fileCount;
};

#endif // TCPSERVERTHREAD_H
