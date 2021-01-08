#ifndef SQLITE_STORAGEIMPL_H
#define SQLITE_STORAGEIMPL_H
#include "storage.h"
#include "feeddatabase.h"
#include "uniquefactory.h"
#include "feedref.h"

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
    FeedCore::Future<FeedCore::ArticleRef> *updateArticleRead(ArticleImpl *article, bool isRead);
    FeedCore::Future<FeedCore::ArticleRef> *storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item);
    void updateFeedMetadata(FeedImpl *feed);
    FeedCore::Future<FeedCore::ArticleRef> *getAll() final;
    FeedCore::Future<FeedCore::ArticleRef> *getUnread() final;
    FeedCore::Future<FeedCore::FeedRef> *getFeeds() final;
    FeedCore::Future<FeedCore::FeedRef> *storeFeed(const QUrl &url) final;
private:
    FeedDatabase m_db;
    FeedCore::UniqueFactory<qint64, FeedImpl> m_feedFactory;
    FeedCore::UniqueFactory<qint64, ArticleImpl> m_articleFactory;
    void appendFeedResults(FeedCore::Future<FeedCore::FeedRef> *op, FeedQuery &q);
    void appendArticleResults(FeedCore::Future<FeedCore::ArticleRef> *op, ItemQuery &q);
};
}
#endif // SQLITE_STORAGEIMPL_H
