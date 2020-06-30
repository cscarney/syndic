#ifndef FEEDITEMHEADERS_H
#define FEEDITEMHEADERS_H
#include <QString>
#include <QDateTime>
#include <QUrl>

struct FeedItemHeaders {
    QString headline;
    QString author;
    QDateTime date;
    QUrl url;
};

#endif // FEEDITEMHEADERS_H
