#include "sqlitefeed.h"

#include <QVariant>
#include <QSqlQuery>

#include "sqlitefeedstorage.h"
#include "article.h"

namespace FeedCore {

SqliteFeed::SqliteFeed::SqliteFeed(SqliteFeedStorage *storage, qint64 feedId):
    m_storage(storage),
    m_id(feedId)
{
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

ItemQuery *SqliteFeed::startItemQuery(bool unreadFilter)
{
    if (unreadFilter) {
        return m_storage->getUnreadByFeed(this);
    } else {
        return m_storage->getByFeed(this);
    }
}

void SqliteFeed::updateFromSource(const Syndication::FeedPtr &source)
{
    setName(source->title());
    const auto &items = source->items();
    for (const auto &item : items) {
        auto *q = m_storage->storeItem(this, item);
        QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
            for (const auto &item : q->result()) {
                if (!item->isRead()){
                    incrementUnreadCount();
                }
                emit itemAdded(item);
            }
        });
    }
}

void SqliteFeed::setName(const QString &name)
{
    if (populateName(name)) {
        m_storage->updateFeedMetadata(this);
    }
}

void SqliteFeed::setItemRead(SqliteArticle *item, bool isRead)
{
    auto *q = m_storage->updateItemRead(item, isRead);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        const auto &changedItems = q->result();
        for (const auto &changedItem : changedItems) {
            if (changedItem->isRead()) {
                decrementUnreadCount();
            } else {
                incrementUnreadCount();
            }
        }
    });
}

QSharedPointer<SqliteFeed> SqliteFeed::forId(SqliteFeedStorage *storage, qint64 feedId)
{
    static QHash<qint64, QWeakPointer<SqliteFeed>> instances;
    auto &instance = instances[feedId];
    if (instance.isNull()) {
        auto newFeed = QSharedPointer<SqliteFeed>(new SqliteFeed(storage, feedId));
        instance = newFeed;
        return newFeed;
    }
    return instance.toStrongRef();
}

QSharedPointer<SqliteFeed> SqliteFeed::fromQuery(SqliteFeedStorage *storage, const QSqlQuery &q)
{
    qint64 id = q.value(0).toLongLong();
    const auto &result = forId(storage, id);
    result->updateFromQuery(q);
    return result;
}

qint64 SqliteFeed::id() const
{
    return m_id;
}

}
