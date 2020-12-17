#include "sqlitefeedstorage.h"

#include <QVector>
#include <QTimer>
#include <QDebug>
#include <Syndication/Person>

#include "sqlitefeed.h"

namespace FeedCore {

SqliteFeedStorage::SqliteFeedStorage() = default;

template<typename OperationType, typename Callable>
static inline OperationType *doAsync(Callable call)
{
    auto *op = new OperationType;
    QTimer::singleShot(0, op, [op, call]{
        call(op);
        op->finished();
        delete op;
    });
    return op;
}

static inline qint64 feedId(const FeedRef &feed)
{
    return feed.staticCast<SqliteFeed>()->id();
}

ItemQuery *SqliteFeedStorage::getAll()
{
    return doAsync<ItemQuery>([this](auto *op){
        op->setResult( m_db.selectAllItems() );
    });
}

ItemQuery *SqliteFeedStorage::getUnread()
{
    return doAsync<ItemQuery>([this](auto *op){
        op->setResult(  m_db.selectUnreadItems() );
    });
}

ItemQuery *SqliteFeedStorage::getById(qint64 id)
{
    return doAsync<ItemQuery>([this, id](auto *op){
        op->setResult( m_db.selectItem(id) );
    });
}

ItemQuery *SqliteFeedStorage::getByFeed(FeedRef feed)
{
    return doAsync<ItemQuery>([this, feed](auto *op){
        op->setResult(  m_db.selectItemsByFeed(feedId(feed)) );
    });
}

ItemQuery *SqliteFeedStorage::getUnreadByFeed(FeedRef feed)
{
    return doAsync<ItemQuery>([this, feed](auto *op){
        op->setResult(  m_db.selectUnreadItemsByFeed(feedId(feed)) );
    });
}

static inline bool upsertItem(FeedDatabase &db, StoredItem &item)
{
    const auto &itemId = db.selectItemId(feedId(item.feedId), item.localId);
    if (!itemId) {
        db.insertItem(item);
        return true;
    }
    item.id = *itemId;
    db.updateItemHeaders(item.id, item.headers);
    if ((!item.content.isNull()) && (!item.content.isEmpty())) {
        db.updateItemContent(item.id, item.content);
    }
    return false;
}

ItemQuery *SqliteFeedStorage::storeItem(FeedRef feed, const Syndication::ItemPtr &item)
{
    return doAsync<ItemQuery>([this, item, feed](auto *op){
        auto result = m_db.makeStoredItem(item, feed);
        if (upsertItem(m_db, result)) {
            op->setResult( result );
        } else {
            op->setResult();
        }
    });
}

ItemQuery *SqliteFeedStorage::updateItemRead(qint64 itemId, bool isRead)
{
    return doAsync<ItemQuery>([this, itemId, isRead](auto *op){
        auto item = m_db.selectItem(itemId);
        if (item.status.isRead == isRead) {
            op->setResult() ;
        } else {
            m_db.updateItemRead(itemId, isRead);
            item.status.isRead = isRead;
            op->setResult( item );
        }
    });
}

ItemQuery *SqliteFeedStorage::updateItemStarred(qint64 itemId, bool isStarred)
{
    return doAsync<ItemQuery>([this, itemId, isStarred](auto *op){
        auto item = m_db.selectItem(itemId);
        if (item.status.isStarred == isStarred) {
            op->setResult();
        } else {
            m_db.updateItemStarred(itemId, isStarred);
            item.status.isStarred = isStarred;
            op->setResult( item );
        }
    });
}

static inline void appendFeedResults(FeedQuery *op, QSqlQuery &q)
{
    while (q.next()) {
        op->appendResult(SqliteFeed::fromQuery(q));
    }
}

FeedQuery *SqliteFeedStorage::getFeeds()
{
    return doAsync<FeedQuery>([this](auto *op){
        auto q =  m_db.selectAllFeeds();
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
        auto result = m_db.selectFeed(*insertId);
        appendFeedResults(op, result);
    });
}

FeedQuery *SqliteFeedStorage::updateFeed(FeedRef &storedFeed, const Syndication::FeedPtr &update)
{
    return doAsync<FeedQuery>([this, storedFeed, update](auto *op){
        const auto &newName = update->title();
        if (storedFeed->name() != newName) {
            storedFeed->setName(newName);
            m_db.updateFeed(feedId(storedFeed), newName);
            op->setResult( storedFeed );
        } else {
            op->setResult();
        }
    });
}

}
