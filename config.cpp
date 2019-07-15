#include "config.h"
#include <QDebug>

Config::Config(QObject *parent) : QObject(parent),config("config.json")
{

    if(!config.exists())
    {
        configJSON.insert("default_receive_path",QDir::currentPath()+"/receive");
        configJSON.insert("default_port","6868");
        configJSON.insert("disk_cache_size",4096);
        configJSON.insert("permission_mode",0);
        if(!config.open(QIODevice::WriteOnly))
            throw QString("Faild to creat config.json");
        configJSONDoc.setObject(configJSON);
        config.write(configJSONDoc.toJson());
        config.close();
    }
    if(!load())
        throw QString("Faild to load config.json");
}

Config::~Config()
{
    save();
}

bool Config::load()
{
    if(!config.open(QIODevice::ReadOnly))
        return false;
    configJSONDoc=QJsonDocument::fromJson(config.readAll(),&jsonError);
    if(jsonError.error!=QJsonParseError::NoError)
        return false;
    configJSON = configJSONDoc.object();
    defaultPort=configJSON["default_port"].toString();
    defaultRecvPath=configJSON["default_receive_path"].toString();
    diskCacheSize=configJSON["disk_cache_size"].toInt();
    permissionMode=configJSON["permission_mode"].toInt();
    config.close();
    return true;
}

bool Config::save()
{
    configJSON.insert("default_receive_path",defaultRecvPath);
    configJSON.insert("default_port",defaultPort);
    configJSON.insert("disk_cache_size",diskCacheSize);
    configJSON.insert("permission_mode",permissionMode);
    if(!config.open(QIODevice::WriteOnly))
        return false;
    configJSONDoc.setObject(configJSON);
    config.write(configJSONDoc.toJson());
    config.close();
    return true;
}

void Config::settingChanged()
{
    emit changeSetting();
}
