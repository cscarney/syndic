#ifndef FEEDSTORAGE_H
#define FEEDSTORAGE_H

#include <QObject>
#include "feedsource.h"

class FeedStorage : public QObject
{
    Q_OBJECT
public:
    explicit FeedStorage(QObject *parent = nullptr) : QObject(parent) {};
    virtual FeedSource::Item getById(qint64 id) = 0;
    virtual qint64 storeItem(FeedSource::Item const &item) = 0;
    virtual QList<FeedSource::Item> getAll() = 0;

    virtual bool storeFeed(FeedSource::Feed) { return 0; };
    virtual QList<FeedSource::Feed> getFeeds() { return {}; };
};

#endif // FEEDSTORAGE_H
