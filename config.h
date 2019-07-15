#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTranslator>

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
    void changeLanguage();
public slots:
    void settingChanged();
    void languageChanged();
public:
    QString defaultRecvPath;
    QString defaultPort;
    qint64 diskCacheSize;
    int permissionMode;
    QString language;

private:
    QFile config;
    QJsonObject configJSON;
    QJsonParseError jsonError;
    QJsonDocument configJSONDoc;
};

extern Config config;
extern QTranslator translator;

#endif // CONFIG_H
