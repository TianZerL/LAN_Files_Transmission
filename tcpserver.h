#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDir>

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

    QDir getPath() const;
    void setPath(const QDir &value);

signals:
    void confirmResult(bool,QDir);
    void ProgressBarValue(int);
    void error(QTcpSocket::SocketError);

private slots:
    void confirmForReadData(QString IP,QString fileName,qint64 fileSize);
    void progressBarValueForUi(int value);
    void transferError(QTcpSocket::SocketError errornum);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    QDir path;
};

#endif // TCPSERVER_H
