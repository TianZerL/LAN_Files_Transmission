#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    ~Config();
    bool load();
    bool save();

signals:
    void changeSetting();
public slots:
    void settingChanged();
public:
    QString defaultRecvPath;
    QString defaultPort;
    qint64 diskCacheSize;
    int permissionMode;

private:
    QFile config;
    QJsonObject configJSON;
    QJsonParseError jsonError;
    QJsonDocument configJSONDoc;
};

extern Config config;

#endif // CONFIG_H
