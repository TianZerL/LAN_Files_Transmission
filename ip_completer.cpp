#include "ip_completer.h"

IP_Completer::IP_Completer(QObject *parent) : QObject(parent)
{
    ipCompleter.setFilterMode(Qt::MatchStartsWith);
    ipCompleter.setCompletionMode(QCompleter::PopupCompletion);
    ipList<<"127.0.0.1"<<"192.168.137.177"<<"192.168.137.104";
    listModel = new QStringListModel(ipList,parent);
    ipCompleter.setModel(listModel);
}

IP_Completer::~IP_Completer()
{
    delete listModel;
}

QCompleter *IP_Completer::getCompleter()
{
    return &ipCompleter;
}

void IP_Completer::addIP(QString IP)
{
    ipList<<IP;
    listModel->setStringList(ipList);
}
