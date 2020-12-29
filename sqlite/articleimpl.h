#ifndef SQLITEARTICLE_H
#define SQLITEARTICLE_H
#include "article.h"
#include "uniquefactory.h"
class QSqlQuery;

namespace Sqlite {
class FeedImpl;
class ItemQuery;

class ArticleImpl : public FeedCore::Article
{
    Q_OBJECT
public:
    qint64 id() const;
    void updateFromQuery(const ItemQuery &q);
    void requestContent() final;
    void setRead(bool isRead) final;
private:
    ArticleImpl(qint64 id, const QSharedPointer<FeedImpl> &feed);
    qint64 m_id;
    QSharedPointer<FeedImpl> m_feed;
    QString m_content;
    friend FeedCore::UniqueFactory<qint64, ArticleImpl>;
};
}
#endif // SQLITEARTICLE_H
