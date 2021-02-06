#ifndef SQLITE_ARTICLEIMPL_H
#define SQLITE_ARTICLEIMPL_H
#include "article.h"
#include "factory.h"
class QSqlQuery;

namespace Sqlite {
class FeedImpl;
class StorageImpl;
class ItemQuery;

class ArticleImpl : public FeedCore::Article
{
    Q_OBJECT
public:
    qint64 id() const;
    void updateFromQuery(const ItemQuery &q);
    void requestContent() final;
private:
    ArticleImpl(qint64 id, StorageImpl *storage, FeedImpl *feed, const ItemQuery &q);
    qint64 m_id;
    QString m_content;
    friend FeedCore::SharedFactory<qint64, ArticleImpl>;
};
}
#endif // SQLITE_ARTICLEIMPL_H
