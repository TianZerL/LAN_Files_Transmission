#ifndef IP_COMPLETER_H
#define IP_COMPLETER_H

#include <QObject>
#include <QCompleter>
#include <QStringListModel>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

class IP_Completer : public QObject
{
    Q_OBJECT
public:
    explicit IP_Completer(QObject *parent = nullptr);
    ~IP_Completer();
    QCompleter* getCompleter();
    bool saveHistory();
    bool loadHistory();


signals:

public slots:
    void addIP(QString IP);

private:
    QFile ipHistory;
    QJsonArray ipListJSON;
    QJsonDocument ipListJSONDoc;
    QStringList ipList;
    QJsonParseError jsonError;
    QStringListModel *listModel;
    QCompleter ipCompleter;
};

#endif // IP_COMPLETER_H
