#ifndef SQLITEFEED_H
#define SQLITEFEED_H

#include "feed.h"

class QSqlQuery;

namespace FeedCore {

class SqliteFeedStorage;
class SqliteArticle;
class XMLFeedUpdater;

class SqliteFeed : public Feed {
    Q_OBJECT
public:
    static QSharedPointer<SqliteFeed> forId(SqliteFeedStorage *storage, qint64 feedId);
    static QSharedPointer<SqliteFeed> fromQuery(SqliteFeedStorage *storage, const QSqlQuery &q);

    qint64 id() const;

    void updateFromQuery(const QSqlQuery &query);
    void populateNew(const QUrl &url, const QString &name);
    void setItemRead(SqliteArticle *item, bool isRead);

    ItemQuery *startItemQuery(bool unreadFilter) final;
    void updateFromSource(const Syndication::FeedPtr &source) final;
    FeedUpdater *updater() final;
    void setName(const QString &name) final;

private:
    SqliteFeed(SqliteFeedStorage *storage, qint64 feedId);

    SqliteFeedStorage *m_storage;
    qint64 m_id;
    XMLFeedUpdater *m_updater;
};

}

#endif // SQLITEFEED_H
