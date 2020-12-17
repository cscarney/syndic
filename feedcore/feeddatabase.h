#ifndef FEEDDATABASE_H
#define FEEDDATABASE_H

#include <QDateTime>
#include <QUrl>
#include <QSqlQuery>

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
    QVector<StoredItem> selectItemsByFeed(qint64 feedId);
    QVector<StoredItem> selectUnreadItemsByFeed(qint64 feedId);
    StoredItem selectItem(qint64 id);
    StoredItem selectItem(qint64 feed, const QString &localId);
    std::optional<qint64> selectItemId(qint64 feedId, const QString &localId);

    void insertItem(StoredItem &item);
    void updateItemHeaders(qint64 id, FeedItemHeaders const &headers);
    void updateItemContent(qint64 id, const QString &content);
    void updateItemRead(qint64 id, bool isRead);
    void updateItemStarred(qint64 id, bool isStarred);

    QSqlQuery selectAllFeeds();
    QSqlQuery selectFeed(qint64 feedId);
    std::optional<qint64> selectFeedId(qint64 source, const QString &localId);
    std::optional<qint64> insertFeed(const QUrl& url);
    bool updateFeed(qint64 feedId, const QString &name);

    // TODO get rid of this
    static StoredItem makeStoredItem(const Syndication::ItemPtr &item, const FeedRef &feed);
};

}

#endif // FEEDDATABASE_H
