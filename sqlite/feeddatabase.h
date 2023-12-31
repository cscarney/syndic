/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "sqlite/feedquery.h"
#include "sqlite/itemquery.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QUrl>
#include <optional>

namespace SqliteStorage
{
class FeedDatabase
{
public:
    explicit FeedDatabase(const QString &filePath);
    ~FeedDatabase();
    FeedDatabase(const FeedDatabase &) = delete;
    FeedDatabase &operator=(const FeedDatabase &) = delete;
    ItemQuery selectAllItems();
    ItemQuery selectUnreadItems();
    ItemQuery selectStarredItems();
    ItemQuery selectItemsBySearch(const QString &search);
    static constexpr const int kDefaultNumberOfRecommendedItems = 20;
    ItemQuery selectItemsByRecommended(int limit = kDefaultNumberOfRecommendedItems);
    ItemQuery selectItemsByFeed(qint64 feedId);
    ItemQuery selectUnreadItemsByFeed(qint64 feedId);
    ItemQuery selectItem(qint64 id);
    ItemQuery selectItem(qint64 feed, const QString &localId);
    QString selectItemContent(qint64 id);
    QString selectItemReadableContent(qint64 id);
    std::optional<qint64> selectItemId(qint64 feedId, const QString &localId);
    std::optional<qint64>
    insertItem(qint64 feedId, const QString &localId, const QString &title, const QString &author, time_t date, const QUrl &url, const QString &content);
    void updateItemHeaders(qint64 id, const QString &title, const QString &author, const QUrl &url);
    void updateItemDate(qint64 id, time_t date);
    void updateItemContent(qint64 id, const QString &content);
    void updateItemReadableContent(qint64 id, const QString &readableContent);
    void updateItemRead(qint64 id, bool isRead);
    void updateItemStarred(qint64 id, bool isStarred);
    void deleteItemsForFeed(qint64 feedId);
    void deleteItemsOlderThan(qint64 feedId, const QDateTime &olderThan);

    FeedQuery selectAllFeeds();
    FeedQuery selectFeed(qint64 feedId);
    std::optional<qint64> insertFeed(const QUrl &url);
    void updateFeedName(qint64 feedId, const QString &name);
    void updateFeedUrl(qint64 feedId, const QUrl &url);
    void updateFeedCategory(qint64 feedId, const QString &category);
    void updateFeedLink(qint64 feedId, const QString &link);
    void updateFeedIcon(qint64 feedId, const QString &icon);
    void updateFeedUpdateInterval(qint64 feedId, qint64 updateInterval);
    void updateFeedLastUpdate(qint64 feedId, const QDateTime &lastUpdated);
    void updateFeedExpireAge(qint64 feedId, qint64 expireAge);
    void updateFeedFlags(qint64 feedId, int flags);
    void deleteFeed(qint64 feedId);

    void beginTransaction();
    void commitTransaction();

private:
    QSqlDatabase db();
    QString m_dbName;
};

}
