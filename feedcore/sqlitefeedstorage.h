#ifndef SQLITEFEEDSTORAGE_H
#define SQLITEFEEDSTORAGE_H

#include "feedstorage.h"
#include "feeddatabase.h"

namespace FeedCore {

class SqliteFeed;
class SqliteArticle;

class SqliteFeedStorage : public FeedStorage
{
    Q_OBJECT
public:
    ItemQuery *getById(qint64 id);
    ItemQuery *getByFeed(SqliteFeed *feedId);
    ItemQuery *getUnreadByFeed(SqliteFeed *feedId);
    ItemQuery *updateItemRead(SqliteArticle *article, bool isRead);
    ItemQuery *storeItem(SqliteFeed *feed, const Syndication::ItemPtr &item);
    void updateFeedMetadata(SqliteFeed *feed);

    // FeedStorage
    ItemQuery *getAll() final;
    ItemQuery *getUnread() final;
    FeedQuery *getFeeds() final;
    FeedQuery *storeFeed(const QUrl &url) final;

private:
    FeedDatabase m_db;

    void appendFeedResults(FeedQuery *op, QSqlQuery &q);
    void appendItemResults(ItemQuery *op, QSqlQuery &q);
};

}

#endif // SQLITEFEEDSTORAGE_H
