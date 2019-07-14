#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDir>

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    enum ServerMode
    {
        SingleThread,MultiThread
    };
    enum PermissionMode
    {
        NeedConfirmation,AlwaysAccept
    };
public:
    explicit TcpServer(QObject *parent = nullptr, ServerMode _serverMode = SingleThread, PermissionMode _permissionMode = NeedConfirmation);

    QDir getPath() const;
    void setPath(const QDir &value);

signals:
    void confirmResult(bool,QDir);
    void ProgressBarValue(int);
    void error(QString);

private slots:
    void confirmForReadData(QString IP,QString fileName,qint64 fileSize);
    void progressBarValueForUi(int value);
    void transferError(QString errorString);
    void finished();
    void resetThreadFlag();

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    QDir path;
    bool threadFlag;
    ServerMode serverMode;
    PermissionMode permissionMode;
};

#endif // TCPSERVER_H
