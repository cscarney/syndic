#ifndef SQLITEFEEDSTORAGE_H
#define SQLITEFEEDSTORAGE_H

#include "feedstorage.h"
#include "feeddatabase.h"

class SqliteFeedStorage : public FeedStorage
{
    Q_OBJECT
public:
    SqliteFeedStorage();

private:
    FeedDatabase m_db;

    // FeedStorage interface
public:
    ItemQuery *getAll() override final;
    ItemQuery *getUnread() override final;
    ItemQuery *getById(qint64 id) override final;
    ItemQuery *getByFeed(qint64 feedId) override final;
    ItemQuery *getUnreadByFeed(qint64 feedId) override final;
    ItemQuery *storeItem(qint64 feedId, Syndication::ItemPtr item) override final;
    ItemQuery *updateItemRead(qint64 itemId, bool isRead) override final;
    ItemQuery *updateItemStarred(qint64 itemId, bool isStarred) override final;

    FeedQuery *getFeeds() override final;
    FeedQuery *storeFeed(QUrl url) override final;
    FeedQuery *updateFeed(qint64 id, Syndication::FeedPtr feed) override final;
};

#endif // SQLITEFEEDSTORAGE_H
