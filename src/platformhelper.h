#ifndef PLATFORMHELPER_H
#define PLATFORMHELPER_H

#include <QObject>
#include <QUrl>

class PlatformHelper : public QObject
{
    Q_OBJECT
public:
    explicit PlatformHelper(QObject *parent = nullptr);
    Q_INVOKABLE void share(const QUrl &url);
};

#endif // PLATFORMHELPER_H
