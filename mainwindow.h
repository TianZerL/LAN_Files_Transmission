#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QByteArray>
#include <QFileDialog>
#include "config.h"
#include "ip_completer.h"
#include "tcpserver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void readyForSendData();
    void waitForConfirm(qint64);
    void addIPToList(QString);

private slots:
    void creat_Connection();

    void read_Data();

    void cls_currConnection();

    void client_Error();

    void server_Error();

    void server_connection_Error();

    void send_Head();

    void confirm_Head(qint64 headSize);

    void start_send_Data();

    void setProgressBar(int value);

    void on_listen_pb_clicked();

    void on_cancle_pb_clicked();

    void on_send_pb_clicked();

    void on_pick_pb_clicked();

    void on_pick_pb_server_clicked();

private:

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    //QList<QTcpSocket *> tcpCLient_List;
    QTcpSocket *currClient;
    QTcpServer *tcpServer;
    IP_Completer *ipCompleter;

    qint64 blockSize;  //单次发送数据大小
    qint64 diskCacheSize;

    QFile *srcFile;
    QFileInfo srcFileInfo;
    QByteArray src_fileCache;
    QString src_fileName;
    qint64 src_fileSize,sended_fileSize,tosend_fileSize;

    QFile *recvFile;
    QDir recvPath;
    QByteArray recv_fileCache;
    QString headInfo,recv_fileName;
    qint64 recv_fileSize, remaining_fileSize;
    bool isHead;

    Config config;
};

#endif // MAINWINDOW_H
