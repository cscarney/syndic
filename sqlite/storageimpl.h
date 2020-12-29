#ifndef SQLITEFEEDSTORAGE_H
#define SQLITEFEEDSTORAGE_H
#include "storage.h"
#include "feeddatabase.h"
namespace FeedCore {
    class FeedRef;
}

namespace Sqlite {
class FeedImpl;
class ArticleImpl;

class StorageImpl : public FeedCore::Storage
{
    Q_OBJECT
public:
    FeedCore::Future<FeedCore::ArticleRef> *getById(qint64 id);
    FeedCore::Future<FeedCore::ArticleRef> *getByFeed(FeedImpl *feedId);
    FeedCore::Future<FeedCore::ArticleRef> *getUnreadByFeed(FeedImpl *feedId);
    FeedCore::Future<FeedCore::ArticleRef> *updateItemRead(ArticleImpl *article, bool isRead);
    FeedCore::Future<FeedCore::ArticleRef> *storeItem(FeedImpl *feed, const Syndication::ItemPtr &item);
    void updateFeedMetadata(FeedImpl *feed);
    FeedCore::Future<FeedCore::ArticleRef> *getAll() final;
    FeedCore::Future<FeedCore::ArticleRef> *getUnread() final;
    FeedCore::Future<FeedCore::FeedRef> *getFeeds() final;
    FeedCore::Future<FeedCore::FeedRef> *storeFeed(const QUrl &url) final;
private:
    FeedDatabase m_db;
    void appendFeedResults(FeedCore::Future<FeedCore::FeedRef> *op, QSqlQuery &q);
    void appendItemResults(FeedCore::Future<FeedCore::ArticleRef> *op, QSqlQuery &q);
};
}
#endif // SQLITEFEEDSTORAGE_H
