#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include "config.h"

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

public:
    enum {
        _4096,_8192,_12288,_16384
    };

private:
    void apply();

signals:
    void settingChanged();

private slots:
    void on_pick_pb_server_clicked();

    void on_apply_pb_clicked();

    void on_ok_pb_clicked();

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H
