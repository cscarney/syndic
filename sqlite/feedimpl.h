#ifndef SQLITEFEED_H
#define SQLITEFEED_H
#include "feed.h"
#include "uniquefactory.h"
class QSqlQuery;
namespace FeedCore {
    class XMLUpdater;
}

namespace Sqlite {
class StorageImpl;
class ArticleImpl;
class FeedQuery;

class FeedImpl : public FeedCore::Feed {
    Q_OBJECT
public:
    qint64 id() const;
    void updateFromQuery(const FeedQuery &query);
    void populateNew(const QUrl &url, const QString &name);
    void setRead(ArticleImpl *article, bool isRead);
    FeedCore::Future<FeedCore::ArticleRef> *getArticles(bool unreadFilter) final;
    void updateFromSource(const Syndication::FeedPtr &source) final;
    FeedCore::Updater *updater() final;
    void setName(const QString &name) final;
private:
    FeedImpl(qint64 feedId, StorageImpl *storage);
    qint64 m_id { 0 };
    StorageImpl *m_storage{ nullptr };
    FeedCore::XMLUpdater *m_updater{ nullptr };
    friend FeedCore::UniqueFactory<qint64, FeedImpl>;
};
}
#endif // SQLITEFEED_H
