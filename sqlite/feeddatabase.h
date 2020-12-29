#ifndef FEEDDATABASE_H
#define FEEDDATABASE_H
#include <QDateTime>
#include <QUrl>
#include <QSqlQuery>
#include <optional>

namespace Sqlite {
class FeedDatabase
{
public:
    FeedDatabase();
    ~FeedDatabase();
    FeedDatabase(const FeedDatabase&) = delete;
    FeedDatabase &operator=(const FeedDatabase&) = delete;
    QSqlQuery selectAllItems();
    QSqlQuery selectUnreadItems();
    QSqlQuery selectItemsByFeed(qint64 feedId);
    QSqlQuery selectUnreadItemsByFeed(qint64 feedId);
    QSqlQuery selectItem(qint64 id);
    QSqlQuery selectItem(qint64 feed, const QString &localId);
    std::optional<qint64> selectItemId(qint64 feedId, const QString &localId);
    std::optional<qint64> insertItem(qint64 feedId, const QString &localId, const QString &title, const QString &author, const QDateTime &date, const QUrl &url, const QString &content);
    void updateItemHeaders(qint64 id, const QString &title, const QDateTime &date, const QString &author, const QUrl &url);
    void updateItemContent(qint64 id, const QString &content);
    void updateItemRead(qint64 id, bool isRead);
    void updateItemStarred(qint64 id, bool isStarred);
    QSqlQuery selectAllFeeds();
    QSqlQuery selectFeed(qint64 feedId);
    std::optional<qint64> selectFeedId(qint64 source, const QString &localId);
    std::optional<qint64> insertFeed(const QUrl& url);
    bool updateFeed(qint64 feedId, const QString &name);
};

}

#endif // FEEDDATABASE_H
