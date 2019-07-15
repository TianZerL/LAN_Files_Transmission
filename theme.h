#ifndef THEME_H
#define THEME_H

#include <QString>

class Theme
{
public:
    Theme();

    static QString setStyle(const QString &path);
};

#endif // THEME_H
