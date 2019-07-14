#include "ip_completer.h"

IP_Completer::IP_Completer(QObject *parent) : QObject(parent), ipHistory("history.json")
{
    ipCompleter.setFilterMode(Qt::MatchStartsWith);
    ipCompleter.setCompletionMode(QCompleter::PopupCompletion);
    if(!ipHistory.exists())
    {
        ipList<<"127.0.0.1";
        if(!saveHistory())
            throw QString("save ip history error");
        ipList.clear();
    }
    if(!loadHistory())
        throw QString("load ip history error");
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

bool IP_Completer::saveHistory()
{
    ipListJSON = QJsonArray::fromStringList(ipList);
    ipListJSONDoc.setArray(ipListJSON);
    if(!ipHistory.open(QIODevice::WriteOnly))
        return false;
    ipHistory.write(ipListJSONDoc.toJson());
    ipHistory.close();
    return true;
}

bool IP_Completer::loadHistory()
{
    if(!ipHistory.open(QIODevice::ReadOnly))
        return false;
    ipListJSONDoc = QJsonDocument::fromJson(ipHistory.readAll(),&jsonError);
    if(jsonError.error!=QJsonParseError::NoError)
        return false;
    ipListJSON = ipListJSONDoc.array();
    for (int i=0;i<ipListJSON.size();i++)
        ipList<<ipListJSON[i].toString();
    ipHistory.close();
    return true;
}

void IP_Completer::addIP(QString IP)
{
    ipList<<IP;
    ipList.removeDuplicates();
    listModel->setStringList(ipList);
}
