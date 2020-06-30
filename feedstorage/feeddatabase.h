#ifndef FEEDDATABASE_H
#define FEEDDATABASE_H

#include <QDateTime>
#include <QUrl>

#include <optional>
#include "feedheaders.h"
#include "feeditemheaders.h"
#include "feeditemstatus.h"
#include "storeditem.h"
#include "storedfeed.h"

class FeedDatabase
{
public:
    explicit FeedDatabase();

    QVector<StoredItem> selectAllItems();
    QVector<StoredItem> selectUnreadItems();
    QVector<StoredItem> selectItemsByFeed(qint64 feedId);
    QVector<StoredItem> selectUnreadItemsByFeed(qint64 feedId);
    StoredItem selectItem(qint64 id);
    StoredItem selectItem(qint64 feed, QString localId);
    std::optional<qint64> selectItemId(qint64 feed, QString localId);

    void insertItem(StoredItem &item);
    void updateItemHeaders(qint64 id, FeedItemHeaders const &headers);
    void updateItemContent(qint64 id, QString content);
    void updateItemRead(qint64 id, bool isRead);
    void updateItemStarred(qint64 id, bool isStarred);

    QVector<StoredFeed> selectAllFeeds();
    StoredFeed selectFeed(qint64 feedId);
    StoredFeed selectFeed(qint64 source, QString localId);
    std::optional<qint64> selectFeedId(qint64 source, QString localId);
    void insertFeed(StoredFeed &feed);
};

#endif // FEEDDATABASE_H
