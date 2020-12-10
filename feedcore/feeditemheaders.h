#ifndef FEEDITEMHEADERS_H
#define FEEDITEMHEADERS_H
#include <QString>
#include <QDateTime>
#include <QUrl>

namespace FeedCore {

class FeedItemHeaders {
public:
    QString headline;
    QString author;
    QDateTime date;
    QUrl url;
};

}

#endif // FEEDITEMHEADERS_H
