#include "mainwindow.h"
#include "config.h"
#include "theme.h"
#include <QApplication>

Config config;
QTranslator translator;

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);   
    a.setStyle("fusion");
    if(config.language == "zh_hans")
    {
        translator.load(":/zh_hans.qm");
        a.installTranslator(&translator);
    }
    MainWindow w;
    w.show();

    return a.exec();
}
