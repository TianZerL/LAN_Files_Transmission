#ifndef ADVANCEDMODE_WIDGET_H
#define ADVANCEDMODE_WIDGET_H

#include <QWidget>
#include <QHostInfo>
#include <QTcpSocket>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include "tcpserver.h"
#include "ip_completer.h"
#include "config.h"

namespace Ui {
class advancedmode_widget;
}

class advancedmode_widget : public QWidget
{
    Q_OBJECT

public:
    explicit advancedmode_widget(QWidget *parent = nullptr);
    ~advancedmode_widget();

signals:
    void addIPToList(QString);

private slots:
    void on_lantest_pb_clicked();

    void on_wantest_pb_clicked();

    void lookedUp(QHostInfo hostInfo);

    void send_Head();

    void confirm_Head();

    void client_Error();

    void on_send_pb_clicked();

    void on_pick_pb_clicked();

    void on_cancle_pb_client_clicked();

private:
    void getFileList(const QString &path);

    void start_send_Data();

private:
    Ui::advancedmode_widget *ui;
    QTcpSocket *tcpClient;
    TcpServer *tcpServer;
    IP_Completer *ipCompleter;

    qint64 blockSize;  //单次发送数据大小

    QFileInfoList srcFileList;
    QDir srcPath;
    QFile *currFile;
    QByteArray src_fileCache;
    QString src_fileName;
    qint64 src_pathSize,tosend_pathSize;
    qint64 src_fileSize,sended_fileSize,tosend_fileSize;
    int i;
    bool cancleFlag;
};

#endif // ADVANCEDMODE_WIDGET_H
