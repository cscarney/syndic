#ifndef SQLITEARTICLE_H
#define SQLITEARTICLE_H

#include "article.h"

class QSqlQuery;

namespace FeedCore {

class SqliteFeed;

class SqliteArticle : public FeedCore::Article
{
    Q_OBJECT
public:
    static QSharedPointer<SqliteArticle> forId(const QSharedPointer<SqliteFeed> &feed, qint64 id);
    static QSharedPointer<SqliteArticle> fromQuery(const QSharedPointer<SqliteFeed> &feed, const QSqlQuery &q);

    qint64 id() const;
    SqliteFeed *feedImpl();
    void updateFromQuery(const QSqlQuery &q);
    void requestContent() final;
    void setRead(bool isRead) final;

private:
    SqliteArticle(const QSharedPointer<SqliteFeed> &feed, qint64 id);
    qint64 m_id;
    QString m_content;
};

}

#endif // SQLITEARTICLE_H
