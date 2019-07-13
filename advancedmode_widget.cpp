#include "advancedmode_widget.h"
#include "ui_advancedmode_widget.h"

advancedmode_widget::advancedmode_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::advancedmode_widget)
{
    ui->setupUi(this);

}

advancedmode_widget::~advancedmode_widget()
{
    delete ui;
}
