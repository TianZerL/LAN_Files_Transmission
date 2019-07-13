#include "tcpserver.h"
#include "tcpserverthread.h"
#include <QThread>

TcpServer::TcpServer(QObject *parent):
    QTcpServer (parent)
{

}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    TcpServerThread *thread = new TcpServerThread(socketDescriptor);
    QThread *threadContainer = new QThread(this);
    thread->moveToThread(threadContainer);
    connect(threadContainer,SIGNAL(finished()),thread,SLOT(deleteLater()));
    connect(threadContainer,SIGNAL(finished()),threadContainer,SLOT(deleteLater()));
    connect(thread,SIGNAL(readFinished()),threadContainer,SLOT(quit()));
    connect(threadContainer,SIGNAL(started()),thread,SLOT(inil()));
    threadContainer->start();
}
