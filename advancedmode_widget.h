#ifndef ADVANCEDMODE_WIDGET_H
#define ADVANCEDMODE_WIDGET_H

#include <QWidget>
#include <QHostInfo>

namespace Ui {
class advancedmode_widget;
}

class advancedmode_widget : public QWidget
{
    Q_OBJECT

public:
    explicit advancedmode_widget(QWidget *parent = nullptr);
    ~advancedmode_widget();

private slots:
    void on_lantest_pb_clicked();

    void on_wantest_pb_clicked();

    void lookedUp(QHostInfo hostInfo);

private:
    Ui::advancedmode_widget *ui;
};

#endif // ADVANCEDMODE_WIDGET_H
