#ifndef FEEDSTORAGE_H
#define FEEDSTORAGE_H

#include <QObject>
#include <Syndication/Item>
#include <Syndication/Feed>

#include "feedstorageoperation.h"

namespace FeedCore {

class FeedStorage : public QObject {
    Q_OBJECT
public:
    explicit FeedStorage(QObject *parent = nullptr) : QObject(parent) {};

    virtual ItemQuery *getAll() = 0;
    virtual ItemQuery *getUnread() = 0;
    virtual ItemQuery *getById(qint64 id) = 0;  // id must exist
    virtual ItemQuery *getByFeed(FeedRef feedId) = 0;
    virtual ItemQuery *getUnreadByFeed(FeedRef feedId) = 0;

    virtual ItemQuery *storeItem(FeedRef feedId, const Syndication::ItemPtr &item) = 0;
    virtual ItemQuery *updateItemRead(qint64 itemId, bool isRead) = 0;
    virtual ItemQuery *updateItemStarred(qint64 itemId, bool isStarred) = 0;

    virtual FeedQuery *getFeeds() = 0;
    virtual FeedQuery *storeFeed(const QUrl &url)=0;
    virtual FeedQuery *updateFeed(FeedRef &storedFeed, const Syndication::FeedPtr &update)=0;

    inline ItemQuery *startItemQuery(FeedRef feedFilter, bool unreadOnly)
    {
        if (!feedFilter.isNull()) {
            if (unreadOnly) return getUnreadByFeed(feedFilter);
            else return getByFeed(feedFilter);
        } else {
            if (unreadOnly) return getUnread();
            else return getAll();
        }
    }
};

}

#endif // FEEDSTORAGE_H
