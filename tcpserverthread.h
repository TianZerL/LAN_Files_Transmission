#ifndef TCPSERVERTHREAD_H
#define TCPSERVERTHREAD_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTcpSocket>

class TcpServerThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerThread(qintptr _socketDescriptor,QObject *parent = nullptr);
    ~TcpServerThread();

signals:
    void error(QTcpSocket::SocketError);
    void readFinished();
    void needConfirm(QString,QString,qint64);
    void progress(int);
public slots:
    void inil();
    void readyConfirm();
    void confirm(bool signal,QDir _recvPath);
    void readData();

private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
    QFile *recvFile;
    QDir recvPath;
    QByteArray recv_fileCache;
    QString headInfo,recv_fileName;
    qint64 recv_fileSize, remaining_fileSize;
};

#endif // TCPSERVERTHREAD_H
