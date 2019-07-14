#include "mainwindow.h"
#include "config.h"
#include <QApplication>

Config config;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
