#ifndef SQLITEFEEDSTORAGE_H
#define SQLITEFEEDSTORAGE_H

#include "feedstorage.h"
#include "feeddatabase.h"

namespace FeedCore {

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
    ItemQuery *getByFeed(FeedRef feedId) override final;
    ItemQuery *getUnreadByFeed(FeedRef feedId) override final;
    ItemQuery *storeItem(FeedRef feedId, const Syndication::ItemPtr &item) override final;
    ItemQuery *updateItemRead(qint64 itemId, bool isRead) override final;
    ItemQuery *updateItemStarred(qint64 itemId, bool isStarred) override final;

    FeedQuery *getFeeds() override final;
    FeedQuery *storeFeed(const QUrl &url) override final;
    FeedQuery *updateFeed(FeedRef &storedFeed, const Syndication::FeedPtr &update) override final;
};

}

#endif // SQLITEFEEDSTORAGE_H
