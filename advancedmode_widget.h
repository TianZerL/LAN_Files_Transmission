#ifndef ADVANCEDMODE_WIDGET_H
#define ADVANCEDMODE_WIDGET_H

#include <QWidget>

namespace Ui {
class advancedmode_widget;
}

class advancedmode_widget : public QWidget
{
    Q_OBJECT

public:
    explicit advancedmode_widget(QWidget *parent = nullptr);
    ~advancedmode_widget();

private:
    Ui::advancedmode_widget *ui;
};

#endif // ADVANCEDMODE_WIDGET_H
