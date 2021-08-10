#ifndef SQLITE_FEEDIMPL_H
#define SQLITE_FEEDIMPL_H
#include "updatablefeed.h"
#include "factory.h"
class QSqlQuery;
namespace FeedCore {
    class XMLUpdater;
}

namespace Sqlite {
class StorageImpl;
class ArticleImpl;
class FeedQuery;

class FeedImpl : public FeedCore::UpdatableFeed {
    Q_OBJECT
public:
    qint64 id() const;
    void updateFromQuery(const FeedQuery &query);
    FeedCore::Future<FeedCore::ArticleRef> *getArticles(bool unreadFilter) final;
    bool editable() final { return true; }
    void updateFromSource(const Syndication::FeedPtr &source) final;
    void onArticleReadChanged(ArticleImpl *article);
private:
    FeedImpl(qint64 feedId, StorageImpl *storage);
    qint64 m_id { 0 };
    StorageImpl *m_storage{ nullptr };
    friend FeedCore::ObjectFactory<qint64, FeedImpl>;
};
}
#endif // SQLITE_FEEDIMPL_H
