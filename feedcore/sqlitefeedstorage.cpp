#include "sqlitefeedstorage.h"

#include <QVector>
#include <QTimer>
#include <QDebug>
#include <Syndication/Person>

#include "sqlitefeed.h"
#include "sqlitearticle.h"

namespace FeedCore {

template<typename OperationType, typename Callable>
static inline OperationType *doAsync(Callable call)
{
    auto *op = new OperationType;
    QTimer::singleShot(0, op, [op, call]{
        call(op);
        emit op->finished();
        delete op;
    });
    return op;
}

static inline qint64 feedId(const FeedRef &feed)
{
    return feed.staticCast<SqliteFeed>()->id();
}

void SqliteFeedStorage::appendItemResults(ItemQuery *op, QSqlQuery &q)
{
    while (q.next()) {
        const qint64 feedId = q.value(1).toLongLong();
        const auto &feed = SqliteFeed::forId(this, feedId);
        op->appendResult(SqliteArticle::fromQuery(feed, q));
    }
}

ItemQuery *SqliteFeedStorage::getAll()
{
    return doAsync<ItemQuery>([this](auto *op){
        QSqlQuery q { m_db.selectAllItems() };
        appendItemResults(op, q);
    });
}

ItemQuery *SqliteFeedStorage::getUnread()
{
    return doAsync<ItemQuery>([this](auto *op){
        QSqlQuery q { m_db.selectUnreadItems() };
        appendItemResults(op, q);
    });
}

ItemQuery *SqliteFeedStorage::getById(qint64 id)
{
    return doAsync<ItemQuery>([this, id](auto *op){
        QSqlQuery q { m_db.selectItem(id) };
        appendItemResults(op, q);
    });
}

ItemQuery *SqliteFeedStorage::getByFeed(SqliteFeed *feed)
{
    const qint64 feedId { feed->id() };
    return doAsync<ItemQuery>([this, feedId](auto *op){
        QSqlQuery q = m_db.selectItemsByFeed(feedId);
        appendItemResults(op, q);
    });
}

ItemQuery *SqliteFeedStorage::getUnreadByFeed(SqliteFeed *feed)
{
    const qint64 feedId = feed->id();
    return doAsync<ItemQuery>([this, feedId](auto *op){
        QSqlQuery q { m_db.selectUnreadItemsByFeed(feedId) };
        appendItemResults(op, q);
    });
}

ItemQuery *SqliteFeedStorage::storeItem(SqliteFeed *feed, const Syndication::ItemPtr &item)
{
    const qint64 feedId { feed->id() };
    return doAsync<ItemQuery>([this, item, feedId](auto *op){
        const auto &itemId = m_db.selectItemId(feedId, item->id());
        const auto &authors = item->authors();
        const auto &authorName = authors.empty() ? "" : authors[0]->name();
        const auto &date = QDateTime::fromTime_t(item->dateUpdated());
        auto content = item->content();
        if (content.isEmpty()) {
            content = item->description();
        }

        if (itemId) {
            m_db.updateItemHeaders(*itemId, item->title(), date, authorName, item->link());
            if (!content.isEmpty()) {
                m_db.updateItemContent(*itemId, content);
            }

            // TODO this  sometimes creates an unnecessary item instance
            getById(*itemId); // push the update into any existing item instance

            op->setResult();
            return;
        }

        const auto &newId = m_db.insertItem(feedId, item->id(), item->title(), authorName, date, item->link(), content);
        if (!newId) {
            op->setResult();
            return;
        }
        QSqlQuery result = m_db.selectItem(*newId);
        appendItemResults(op, result);
    });
}

ItemQuery *SqliteFeedStorage::updateItemRead(SqliteArticle *article, bool isRead)
{
    const qint64 itemId { article->id() };
    const bool oldValue { article->isRead() };
    return doAsync<ItemQuery>([this, itemId, isRead, oldValue](auto *op){
        if (oldValue == isRead) {
            op->setResult() ;
        } else {
            m_db.updateItemRead(itemId, isRead);
            QSqlQuery result { m_db.selectItem(itemId) };
            appendItemResults(op, result);
        }
    });
}

void SqliteFeedStorage::appendFeedResults(FeedQuery *op, QSqlQuery &q)
{
    while (q.next()) {
        op->appendResult(SqliteFeed::fromQuery(this, q));
    }
}

FeedQuery *SqliteFeedStorage::getFeeds()
{
    return doAsync<FeedQuery>([this](auto *op){
        QSqlQuery q { m_db.selectAllFeeds() };
        appendFeedResults(op, q);
    });
}

FeedQuery *SqliteFeedStorage::storeFeed(const QUrl &url)
{
    return doAsync<FeedQuery>([this, url](auto *op){
        const auto &existingId = m_db.selectFeedId(0, url.toString());
        if (existingId) {
            qDebug() << "trying to insert feed for " << url << "which already exists";
            op->setResult();
            return;
        }
        const auto &insertId = m_db.insertFeed(url);
        if (!insertId)
        {
            op->setResult();
            return;
        }
        QSqlQuery result { m_db.selectFeed(*insertId) };
        appendFeedResults(op, result);
    });
}

void SqliteFeedStorage::updateFeedMetadata(SqliteFeed *storedFeed)
{
    const qint64 id = storedFeed->id();
    const QString name = storedFeed->name();
    QTimer::singleShot(0, [this, id, name]{
        m_db.updateFeed(id, name);
    });
}

}
