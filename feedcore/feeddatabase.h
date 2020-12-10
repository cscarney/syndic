#ifndef FEEDDATABASE_H
#define FEEDDATABASE_H

#include <QDateTime>
#include <QUrl>

#include <optional>

#include "storeditem.h"
#include "feed.h"

namespace FeedCore {

class FeedDatabase
{
public:
    explicit FeedDatabase();

    QVector<StoredItem> selectAllItems();
    QVector<StoredItem> selectUnreadItems();
    QVector<StoredItem> selectItemsByFeed(const FeedRef &feed);
    QVector<StoredItem> selectUnreadItemsByFeed(const FeedRef &feed);
    StoredItem selectItem(qint64 id);
    StoredItem selectItem(qint64 feed, const QString &localId);
    std::optional<qint64> selectItemId(const FeedRef &feed, const QString &localId);

    void insertItem(StoredItem &item);
    void updateItemHeaders(qint64 id, FeedItemHeaders const &headers);
    void updateItemContent(qint64 id, const QString &content);
    void updateItemRead(qint64 id, bool isRead);
    void updateItemStarred(qint64 id, bool isStarred);

    QVector<FeedRef> selectAllFeeds();
    void selectFeed(const FeedRef& feed);
    std::optional<qint64> selectFeedId(qint64 source, const QString &localId);
    FeedRef insertFeed(const QUrl& url);
    void updateFeed(const FeedRef &feed);

    // TODO get rid of this
    static StoredItem makeStoredItem(const Syndication::ItemPtr &item, const FeedRef &feed);
};

}

#endif // FEEDDATABASE_H
