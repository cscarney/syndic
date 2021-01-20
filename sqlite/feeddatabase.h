#ifndef SQLITE_FEEDDATABASE_H
#define SQLITE_FEEDDATABASE_H
#include <QDateTime>
#include <QUrl>
#include <QSqlQuery>
#include <optional>
#include "sqlite/feedquery.h"
#include "sqlite/itemquery.h"

namespace Sqlite {
class FeedDatabase
{
public:
    FeedDatabase();
    ~FeedDatabase();
    FeedDatabase(const FeedDatabase&) = delete;
    FeedDatabase &operator=(const FeedDatabase&) = delete;
    ItemQuery selectAllItems();
    ItemQuery selectUnreadItems();
    ItemQuery selectItemsByFeed(qint64 feedId);
    ItemQuery selectUnreadItemsByFeed(qint64 feedId);
    ItemQuery selectItem(qint64 id);
    ItemQuery selectItem(qint64 feed, const QString &localId);
    std::optional<qint64> selectItemId(qint64 feedId, const QString &localId);
    std::optional<qint64> insertItem(qint64 feedId, const QString &localId, const QString &title, const QString &author, const QDateTime &date, const QUrl &url, const QString &content);
    void updateItemHeaders(qint64 id, const QString &title, const QDateTime &date, const QString &author, const QUrl &url);
    void updateItemContent(qint64 id, const QString &content);
    void updateItemRead(qint64 id, bool isRead);
    void updateItemStarred(qint64 id, bool isStarred);
    FeedQuery selectAllFeeds();
    FeedQuery selectFeed(qint64 feedId);
    std::optional<qint64> selectFeedId(qint64 source, const QString &localId);
    std::optional<qint64> insertFeed(const QUrl& url);
    void updateFeedName(qint64 feedId, const QString &name);
    void updateFeedUpdateInterval(qint64 feedId, qint64 updateInterval);
    void updateFeedLastUpdate(qint64 feedId, QDateTime lastUpdated);
private:
    QSqlDatabase db();
    QString m_dbName;
};

}

#endif // SQLITE_FEEDDATABASE_H
