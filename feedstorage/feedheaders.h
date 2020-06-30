#ifndef FEEDHEADERS_H
#define FEEDHEADERS_H
#include <QString>
#include <QUrl>

struct FeedHeaders {
    QString name;
    QUrl location;
    QUrl icon;
};

#endif // FEEDHEADERS_H
