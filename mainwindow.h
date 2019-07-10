#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QFile>
#include <QByteArray>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void creat_Connection();

    void read_Data();

    void cls_Connection();

    void start_send_Data();

    void continue_send_Data(qint64 size_of_bytes);

    void on_listen_pb_clicked();

    void on_cancle_pb_clicked();

    void on_send_pb_clicked();

private:


private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    QList<QTcpSocket *> tcpCLient_List;
    QTcpSocket *currClient;
    QTcpServer *tcpServer;

    QFile *file_src;
    QByteArray file_src_cache;
    QString file_name_src;
    qint64 file_size_src,file_size_src_sended,file_size_src_to_send;

    QFile *file_recv;
    QByteArray file_recv_cache;
    QString file_name_recv;
    qint64 file_size_recv, file_size_sended;
};

#endif // MAINWINDOW_H
