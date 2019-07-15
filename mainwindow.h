#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QByteArray>
#include <QFileDialog>
#include "config.h"
#include "ip_completer.h"
#include "tcpserver.h"
#include "advancedmode_widget.h"

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
    void addIPToList(QString);

private slots:
    void creat_Connection();

    void client_Error();

    void server_Error(QString errorString);

    void server_connection_Error();

    void send_Head();

    void confirm_Head();

    void setProgressBar(int value);

    void on_listen_pb_clicked();

    void on_cancle_pb_clicked();

    void on_send_pb_clicked();

    void on_pick_pb_clicked();

    void on_pick_pb_server_clicked();

    void on_cancle_pb_client_clicked();

    void on_actionAdvance_triggered();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void start_send_Data();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    TcpServer *tcpServer;
    IP_Completer *ipCompleter;

    qint64 blockSize;  //单次发送数据大小

    QFile *srcFile;
    QFileInfo srcFileInfo;
    QByteArray src_fileCache;
    QString src_fileName;
    qint64 src_fileSize,sended_fileSize,tosend_fileSize;
    bool cancleFlag;

};

#endif // MAINWINDOW_H
