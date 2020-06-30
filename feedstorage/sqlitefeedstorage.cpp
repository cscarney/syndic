#include "sqlitefeedstorage.h"

#include <QVector>
#include <QTimer>
#include <QDebug>

SqliteFeedStorage::SqliteFeedStorage()
{

}

template<typename OperationType, typename Callable>
static inline OperationType *doAsync(Callable call)
{
    auto op = new OperationType;
    QTimer::singleShot(0, op, [op, call]{
        call(op);
        op->finished();
        delete op;
    });
    return op;
}

FeedStorage::ItemQuery *SqliteFeedStorage::getAll()
{
    return doAsync<ItemQuery>([this](auto op){
        op->result = m_db.selectAllItems();
    });
}

FeedStorage::ItemQuery *SqliteFeedStorage::getUnread()
{
    return doAsync<ItemQuery>([this](auto op){
        op->result = m_db.selectUnreadItems();
    });
}

FeedStorage::ItemQuery *SqliteFeedStorage::getById(qint64 id)
{
    return doAsync<ItemQuery>([this, id](auto op){
        op->result = {m_db.selectItem(id)};
    });
}

FeedStorage::ItemQuery *SqliteFeedStorage::getByFeed(qint64 feedId)
{
    return doAsync<ItemQuery>([this, feedId](auto op){
        op->result = m_db.selectItemsByFeed(feedId);
    });
}

FeedStorage::ItemQuery *SqliteFeedStorage::getUnreadByFeed(qint64 feedId)
{
    return doAsync<ItemQuery>([this, feedId](auto op){
        op->result = m_db.selectUnreadItemsByFeed(feedId);
    });
}

static inline StoredItem makeStoredItem(FeedSource::Item const &item, qint64 feedId)
{
    return {
        .id = 0, // overwritten below
        .feedId = feedId,
        .localId = QString::number(item.id),
        .headers = {
            .headline = item.headline,
            .author = item.author,
            .date = item.date,
            .url = item.url
        },
        .content = item.content,
        .status = {
            .isRead = false,
            .isStarred = false
        }
    };
}

static inline bool upsertItem(FeedDatabase &db, StoredItem &item)
{
    auto itemId = db.selectItemId(item.feedId, item.localId);
    if (!itemId) {
        db.insertItem(item);
        return true;
    } else {
        item.id = *itemId;
        db.updateItemHeaders(item.id, item.headers);
        if ((!item.content.isNull()) && (!item.content.isEmpty()))
            db.updateItemContent(item.id, item.content);
        return false;
    }
}

FeedStorage::ItemQuery *SqliteFeedStorage::storeItem(const FeedSource::Item &item)
{
    return doAsync<ItemQuery>([this, item](auto op){
        auto feedId = m_db.selectFeedId(0, QString::number(item.feedId));
        if (!feedId) {
            qDebug() << "Tried to store item for non-existent feed\n";
            op->result = {};
            return;
        }
        StoredItem result = makeStoredItem(item, *feedId);
        if (upsertItem(m_db, result)) {
            op->result = {result};
        } else {
            op->result = {};
        }
    });
}

FeedStorage::ItemQuery *SqliteFeedStorage::updateItemRead(qint64 itemId, bool isRead)
{
    return doAsync<ItemQuery>([this, itemId, isRead](auto op){
        m_db.updateItemRead(itemId, isRead);
        op->result = {m_db.selectItem(itemId)};
    });
}

FeedStorage::ItemQuery *SqliteFeedStorage::updateItemStarred(qint64 itemId, bool isStarred)
{
    return doAsync<ItemQuery>([this, itemId, isStarred](auto op){
        m_db.updateItemStarred(itemId, isStarred);
        op->result = {m_db.selectItem(itemId)};
    });
}

FeedStorage::FeedQuery *SqliteFeedStorage::getFeeds()
{
    return doAsync<FeedQuery>([this](auto op){
        op->result = m_db.selectAllFeeds();
    });
}

FeedStorage::FeedQuery *SqliteFeedStorage::storeFeed(const FeedSource::Feed &feed)
{
    return doAsync<FeedQuery>([this, feed](auto op){
        auto localId = QString::number(feed.id);
        StoredFeed result = {
            .id = 0, // overwritten below
            .sourceId = 0, // TODO not robust to multiple sources
            .localId = localId,
            .headers = {
                .name = feed.name
            }
        };

        auto id = m_db.selectFeedId(0, localId);
        if (!id) {
            m_db.insertFeed(result);
        } else {
            result.id = *id;
            // TODO update feed headers
        }
        op->result = {result};
    });
}

