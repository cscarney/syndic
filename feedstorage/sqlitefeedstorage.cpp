#include "sqlitefeedstorage.h"

#include <QVector>
#include <QTimer>
#include <QDebug>
#include <Syndication/Person>

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

ItemQuery *SqliteFeedStorage::getAll()
{
    return doAsync<ItemQuery>([this](auto op){
        op->result = m_db.selectAllItems();
    });
}

ItemQuery *SqliteFeedStorage::getUnread()
{
    return doAsync<ItemQuery>([this](auto op){
        op->result = m_db.selectUnreadItems();
    });
}

ItemQuery *SqliteFeedStorage::getById(qint64 id)
{
    return doAsync<ItemQuery>([this, id](auto op){
        op->result = {m_db.selectItem(id)};
    });
}

ItemQuery *SqliteFeedStorage::getByFeed(qint64 feedId)
{
    return doAsync<ItemQuery>([this, feedId](auto op){
        op->result = m_db.selectItemsByFeed(feedId);
    });
}

ItemQuery *SqliteFeedStorage::getUnreadByFeed(qint64 feedId)
{
    return doAsync<ItemQuery>([this, feedId](auto op){
        op->result = m_db.selectUnreadItemsByFeed(feedId);
    });
}

static inline StoredItem makeStoredItem(Syndication::ItemPtr item, qint64 feedId)
{
    auto authors = item->authors();
    auto authorName = authors.empty() ? "" : authors[0]->name();
    auto date = QDateTime::fromTime_t(item->dateUpdated());
    auto content = item->content();
    if (content.isEmpty()) content = item->description();
    return {
        .id = 0, // overwritten below
        .feedId = feedId,
        .localId = item->id(),
        .headers = {
            .headline = item->title(),
            .author = authorName,
            .date = date,
            .url = item->link()
        },
        .content = content,
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

ItemQuery *SqliteFeedStorage::storeItem(qint64 feedId, Syndication::ItemPtr item)
{
    return doAsync<ItemQuery>([this, item, feedId](auto op){
        StoredItem result = makeStoredItem(item, feedId);
        if (upsertItem(m_db, result)) {
            op->result = {result};
        } else {
            op->result = {};
        }
    });
}

ItemQuery *SqliteFeedStorage::updateItemRead(qint64 itemId, bool isRead)
{
    return doAsync<ItemQuery>([this, itemId, isRead](auto op){
        auto item = m_db.selectItem(itemId);
        if (item.status.isRead == isRead) {
            op->result = {};
        } else {
            m_db.updateItemRead(itemId, isRead);
            item.status.isRead = isRead;
            op->result = {item};
        }
    });
}

ItemQuery *SqliteFeedStorage::updateItemStarred(qint64 itemId, bool isStarred)
{
    return doAsync<ItemQuery>([this, itemId, isStarred](auto op){
        auto item = m_db.selectItem(itemId);
        if (item.status.isStarred == isStarred) {
            op->result = {};
        } else {
            m_db.updateItemStarred(itemId, isStarred);
            item.status.isStarred = isStarred;
            op->result = {item};
        }
    });
}

FeedQuery *SqliteFeedStorage::getFeeds()
{
    return doAsync<FeedQuery>([this](auto op){
        op->result = m_db.selectAllFeeds();
    });
}

/*
FeedStorage::FeedQuery *SqliteFeedStorage::storeFeed(const FeedSource::Feed &feed)
{
    return doAsync<FeedQuery>([this, feed](auto op){
        auto localId = QString::number(feed.id);
        StoredFeed result = {
            .id = 0, // overwritten below
            .sourceId = 0, // TODO not robust to multiple sources
            .localId = localId,
            .headers = {
                .name = feed.name,
                .url = feed.url
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
*/
