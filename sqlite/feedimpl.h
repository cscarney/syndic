#ifndef SQLITEFEED_H
#define SQLITEFEED_H
#include "feed.h"
class QSqlQuery;
namespace FeedCore {
    class XMLUpdater;
}

namespace Sqlite {
class StorageImpl;
class ArticleImpl;

class FeedImpl : public FeedCore::Feed {
    Q_OBJECT
public:
    static QSharedPointer<FeedImpl> forId(StorageImpl *storage, qint64 feedId);
    static QSharedPointer<FeedImpl> fromQuery(StorageImpl *storage, const QSqlQuery &q);
    qint64 id() const;
    void updateFromQuery(const QSqlQuery &query);
    void populateNew(const QUrl &url, const QString &name);
    void setItemRead(ArticleImpl *item, bool isRead);
    FeedCore::Future<FeedCore::ArticleRef> *startItemQuery(bool unreadFilter) final;
    void updateFromSource(const Syndication::FeedPtr &source) final;
    FeedCore::Updater *updater() final;
    void setName(const QString &name) final;
private:
    FeedImpl(StorageImpl *storage, qint64 feedId);
    StorageImpl *m_storage;
    qint64 m_id;
    FeedCore::XMLUpdater *m_updater;
};
}
#endif // SQLITEFEED_H
