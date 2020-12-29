#ifndef SQLITEARTICLE_H
#define SQLITEARTICLE_H
#include "article.h"
class QSqlQuery;

namespace Sqlite {
class FeedImpl;

class ArticleImpl : public FeedCore::Article
{
    Q_OBJECT
public:
    static QSharedPointer<ArticleImpl> forId(const QSharedPointer<FeedImpl> &feed, qint64 id);
    static QSharedPointer<ArticleImpl> fromQuery(const QSharedPointer<FeedImpl> &feed, const QSqlQuery &q);
    qint64 id() const;
    FeedImpl *feedImpl();
    void updateFromQuery(const QSqlQuery &q);
    void requestContent() final;
    void setRead(bool isRead) final;
private:
    ArticleImpl(const QSharedPointer<FeedImpl> &feed, qint64 id);
    qint64 m_id;
    QString m_content;
};
}
#endif // SQLITEARTICLE_H
