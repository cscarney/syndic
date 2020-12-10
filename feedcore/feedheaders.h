#ifndef FEEDHEADERS_H
#define FEEDHEADERS_H
#include <QString>
#include <QUrl>

namespace FeedCore {

struct FeedHeaders {
    QString name;
    QUrl url;
};

}

#endif // FEEDHEADERS_H
