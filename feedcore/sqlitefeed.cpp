#include "sqlitefeed.h"

#include <QVariant>
#include <QSqlQuery>

namespace FeedCore {

SqliteFeed::SqliteFeed::SqliteFeed(qint64 feedId)
{
    m_id = feedId;
}

void SqliteFeed::updateFromQuery(const QSqlQuery &query)
{
    populateName(query.value(3).toString());
    populateUrl(query.value(4).toString());
    populateUnreadCount(query.value(5).toInt());
}

void SqliteFeed::populateNew(const QUrl &url, const QString &name)
{
    populateName(name);
    populateUrl(url);
    populateUnreadCount(0);
}

QSharedPointer<SqliteFeed> SqliteFeed::forId(qint64 feedId)
{
    static QHash<qint64, QWeakPointer<SqliteFeed>> instances;
    auto &instance = instances[feedId];
    if (instance.isNull()) {
        auto newFeed = QSharedPointer<SqliteFeed>(new SqliteFeed(feedId));
        instance = newFeed;
        return newFeed;
    }
    return instance.toStrongRef();
}

QSharedPointer<SqliteFeed> SqliteFeed::fromQuery(const QSqlQuery &q)
{
    qint64 id = q.value(0).toLongLong();
    const auto &result = forId(id);
    result->updateFromQuery(q);
    return result;
}

qint64 SqliteFeed::id()
{
    return m_id;
}

}
