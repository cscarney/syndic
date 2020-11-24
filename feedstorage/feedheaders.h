#ifndef FEEDHEADERS_H
#define FEEDHEADERS_H
#include <QString>
#include <QUrl>

struct FeedHeaders {
    QString name;
    QUrl url;
    QUrl icon;
};

#endif // FEEDHEADERS_H
