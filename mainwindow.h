#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>

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

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    QTcpSocket *tcpConnection;
    QTcpServer *tcpServer;
};

#endif // MAINWINDOW_H
