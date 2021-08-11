/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
    explicit FeedDatabase(const QString& filePath);
    ~FeedDatabase();
    FeedDatabase(const FeedDatabase&) = delete;
    FeedDatabase &operator=(const FeedDatabase&) = delete;
    ItemQuery selectAllItems();
    ItemQuery selectUnreadItems();
    ItemQuery selectStarredItems();
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
    void deleteItemsForFeed(qint64 feedId);
    void deleteItemsOlderThan(qint64 feedId, const QDateTime& olderThan);

    FeedQuery selectAllFeeds();
    FeedQuery selectFeed(qint64 feedId);
    std::optional<qint64> selectFeedId(qint64 source, const QString &localId);
    std::optional<qint64> insertFeed(const QUrl& url);
    void updateFeedName(qint64 feedId, const QString &name);
    void updateFeedLink(qint64 feedId, const QString &link);
    void updateFeedIcon(qint64 feedId, const QString &icon);
    void updateFeedUpdateInterval(qint64 feedId, qint64 updateInterval);
    void updateFeedLastUpdate(qint64 feedId, const QDateTime& lastUpdated);
    void deleteFeed(qint64 feedId);
private:
    QSqlDatabase db();
    QString m_dbName;
};

}

#endif // SQLITE_FEEDDATABASE_H
