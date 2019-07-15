#include "theme.h"
#include <QFile>

Theme::Theme()
{

}

QString Theme::setStyle(const QString &path)
{
    QFile qss(path);
    qss.open(QFile::ReadOnly);
    QString qsscontent = qss.readAll();
    qss.close();
    return qsscontent;
}
