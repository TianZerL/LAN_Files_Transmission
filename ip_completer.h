#ifndef IP_COMPLETER_H
#define IP_COMPLETER_H

#include <QObject>
#include <QCompleter>
#include <QStringListModel>

class IP_Completer : public QObject
{
    Q_OBJECT
public:
    explicit IP_Completer(QObject *parent = nullptr);
    ~IP_Completer();
    QCompleter* getCompleter();


signals:

public slots:
    void addIP(QString IP);

private:
    QStringList ipList;
    QStringListModel *listModel;
    QCompleter ipCompleter;
};

#endif // IP_COMPLETER_H
