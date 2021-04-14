#ifndef SQLITE_STORAGEIMPL_H
#define SQLITE_STORAGEIMPL_H
#include "storage.h"
#include "feeddatabase.h"
#include "factory.h"

namespace Sqlite {
class FeedImpl;
class ArticleImpl;

class StorageImpl : public FeedCore::Storage
{
    Q_OBJECT
public:
    explicit StorageImpl(QString filePath);
    FeedCore::Future<FeedCore::ArticleRef> *getById(qint64 id);
    FeedCore::Future<FeedCore::ArticleRef> *getByFeed(FeedImpl *feedId);
    FeedCore::Future<FeedCore::ArticleRef> *getUnreadByFeed(FeedImpl *feedId);
    FeedCore::Future<FeedCore::ArticleRef> *storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item);
    void onArticleReadChanged(ArticleImpl *article);
    void onArticleStarredChanged(ArticleImpl *article);

    FeedCore::Future<FeedCore::ArticleRef> *getAll() final;
    FeedCore::Future<FeedCore::ArticleRef> *getUnread() final;
    FeedCore::Future<FeedCore::ArticleRef> *getStarred() final;
    FeedCore::Future<FeedCore::Feed*> *getFeeds() final;
    FeedCore::Future<FeedCore::Feed*> *storeFeed(FeedCore::Feed *feed) final;
    void listenForChanges(FeedImpl *feed);
private:
    FeedDatabase m_db;
    FeedCore::ObjectFactory<qint64, FeedImpl> m_feedFactory;
    FeedCore::SharedFactory<qint64, ArticleImpl> m_articleFactory;
    void appendFeedResults(FeedCore::Future<FeedCore::Feed*> *op, FeedQuery &q);
    void appendArticleResults(FeedCore::Future<FeedCore::ArticleRef> *op, ItemQuery &q);
    void onFeedRequestDelete(FeedImpl *feed);
};
}
#endif // SQLITE_STORAGEIMPL_H
