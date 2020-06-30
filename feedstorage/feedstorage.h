#ifndef FEEDSTORAGE_H
#define FEEDSTORAGE_H

#include <QObject>
#include "feedsource.h"
#include "feeddatabase.h"

class FeedStorageOperation: public QObject {
    Q_OBJECT

signals:
    void finished();
};


class FeedStorage : public QObject {
    Q_OBJECT
public:
    explicit FeedStorage(QObject *parent = nullptr) : QObject(parent) {};

    class ItemQuery: public FeedStorageOperation {
    public:
        QVector<StoredItem> result;
    };

    class FeedQuery : public FeedStorageOperation {
    public:
        QVector<StoredFeed> result;
    };

    virtual ItemQuery *getAll() = 0;
    virtual ItemQuery *getUnread() = 0;
    virtual ItemQuery *getById(qint64 id) = 0;  // id must exist
    virtual ItemQuery *getByFeed(qint64 feedId) = 0;
    virtual ItemQuery *getUnreadByFeed(qint64 feedId) = 0;

    virtual ItemQuery *storeItem(FeedSource::Item const &item) = 0; /* TODO use KSyndication types */
    virtual ItemQuery *updateItemRead(qint64 itemId, bool isRead) = 0;
    virtual ItemQuery *updateItemStarred(qint64 itemId, bool isStarred) = 0;

    virtual FeedQuery *getFeeds() = 0;
    virtual FeedQuery *storeFeed(FeedSource::Feed const &feed) = 0; /* TODO use KSyndication types */
};

#endif // FEEDSTORAGE_H
