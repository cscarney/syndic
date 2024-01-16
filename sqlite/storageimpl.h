/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "factory.h"
#include "feeddatabase.h"
#include "storage.h"

namespace SqliteStorage
{
class FeedImpl;
class ArticleImpl;

class StorageImpl : public FeedCore::Storage
{
    Q_OBJECT
public:
    explicit StorageImpl(const QString &filePath);
    ~StorageImpl();
    QFuture<FeedCore::ArticleRef> getById(qint64 id);
    QFuture<FeedCore::ArticleRef> getByFeed(FeedImpl *feedId);
    QFuture<FeedCore::ArticleRef> getUnreadByFeed(FeedImpl *feedId);
    QFuture<FeedCore::ArticleRef> storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item);
    QFuture<QString> getContent(ArticleImpl *article);
    QFuture<QString> getReadableContent(ArticleImpl *article);
    void cacheReadableContent(ArticleImpl *article, const QString &readableContent);
    void onArticleReadChanged(ArticleImpl *article);
    void onArticleStarredChanged(ArticleImpl *article);

    QFuture<FeedCore::ArticleRef> getAll() final;
    QFuture<FeedCore::ArticleRef> getUnread() final;
    QFuture<FeedCore::ArticleRef> getStarred() final;
    QFuture<FeedCore::ArticleRef> getSearchResults(const QString &search) override;
    QFuture<FeedCore::ArticleRef> getHighlights(size_t limit) final;
    QFuture<FeedCore::Feed *> getFeeds() final;
    QFuture<FeedCore::Feed *> storeFeed(FeedCore::Feed *feed) final;
    void listenForChanges(FeedImpl *feed);
    void expire(FeedImpl *feed, const QDateTime &olderThan);

private:
    class WorkerThread;
    class Worker;
    WorkerThread *m_thread;
    Worker *m_worker;
    void onFeedRequestDelete(FeedImpl *feed);
    void onUpdateIntervalChanged(FeedImpl *feed);
    void onExpireModeChanged(FeedImpl *feed);
    void onExpireAgeChanged(FeedImpl *feed);
    void onUpdateModeChanged(FeedImpl *feed);
};
}
