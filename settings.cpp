#include "settings.h"
#include "ui_settings.h"
#include <QFileDialog>

Settings::Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->port_le_server->setValidator(new QRegExpValidator(QRegExp("^(?:[0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$"),ui->port_le_server));

    ui->port_le_server->setText(config.defaultPort);
    ui->path_le_server->setText(config.defaultRecvPath);
    switch (config.diskCacheSize)
    {
    case 4096:
        ui->cache_cb->setCurrentIndex(0);
        break;
    case 8192:
        ui->cache_cb->setCurrentIndex(1);
        break;
    case 12288:
        ui->cache_cb->setCurrentIndex(2);
        break;
    case 16384:
        ui->cache_cb->setCurrentIndex(3);
        break;
    }
    switch (config.permissionMode)
    {
    case 0:
        ui->yes_rb->setChecked(true);
        break;
    case 1:
        ui->no_rb->setChecked(true);
        break;
    }
    if(config.language == "en")
        ui->language_cb->setCurrentIndex(0);
    else if(config.language == "zh_hans")
        ui->language_cb->setCurrentIndex(1);

    connect(this,SIGNAL(settingChanged()),&config,SLOT(settingChanged()));
    connect(ui->language_cb,SIGNAL(currentIndexChanged(int)),this,SLOT(languageChanged(int)));
    connect(this,SIGNAL(langChanged()),&config,SLOT(languageChanged()));
}

Settings::~Settings()
{
    delete ui;
}

void Settings::apply()
{
    config.defaultRecvPath = ui->path_le_server->text();
    config.defaultPort = ui->port_le_server->text();
    switch (ui->cache_cb->currentIndex())
    {
    case 0:
        config.diskCacheSize = 4096;
        break;
    case 1:
        config.diskCacheSize = 8192;
        break;
    case 2:
        config.diskCacheSize = 12288;
        break;
    case 3:
        config.diskCacheSize = 16384;
        break;
    }
    if(ui->yes_rb->isChecked())
        config.permissionMode=0;
    else if(ui->no_rb->isChecked())
        config.permissionMode=1;

    switch(ui->language_cb->currentIndex())
    {
    case 0:
        config.language = "en";
        break;
    case 1:
        config.language = "zh_hans";
        break;
    }
    emit settingChanged();
}

void Settings::on_pick_pb_server_clicked()
{
    ui->path_le_server->setText(QFileDialog::getExistingDirectory());
}

void Settings::on_apply_pb_clicked()
{
    apply();
}

void Settings::on_ok_pb_clicked()
{
    apply();
    close();
}

void Settings::languageChanged(int index)
{
    if(index == 1)
    {
        translator.load(":/zh_hans.qm");
        qApp->installTranslator(&translator);
        ui->retranslateUi(this);
        emit langChanged();
    }
    else if(index == 0)
    {
        qApp->removeTranslator(&translator);
        ui->retranslateUi(this);
        emit langChanged();
    }

}
