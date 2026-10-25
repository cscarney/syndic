/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "factory.h"
#include "feed.h"
#include "feeddatabase.h"
#include "storage.h"

namespace SqliteStorage
{
class FeedImpl;
class ArticleImpl;
class StorageImpl;

/**
 * Pseudo-feed for saved pages that aren't associated with a subscription.
 *
 * It doesn't appear in the feed list; its articles are visible through the
 * all items and starred items lists.
 */
class UnfiledFeed : public FeedCore::Feed
{
    Q_OBJECT
public:
    explicit UnfiledFeed(StorageImpl *storage);
    QFuture<FeedCore::ArticleRef> getArticles(bool unreadFilter) final;

private:
    StorageImpl *m_storage;
};

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
    void storeSavedContent(ArticleImpl *article, const QString &readableContent);
    void onArticleReadChanged(ArticleImpl *article);
    void onArticleStarredChanged(ArticleImpl *article);

    QFuture<FeedCore::ArticleRef> getAll() final;
    QFuture<FeedCore::ArticleRef> getUnread() final;
    QFuture<FeedCore::ArticleRef> getStarred() final;
    QFuture<FeedCore::ArticleRef> getSearchResults(const QString &search) override;
    QFuture<FeedCore::ArticleRef> getHighlights(size_t limit) final;
    QFuture<FeedCore::Subscription *> getFeeds() final;
    QFuture<FeedCore::Subscription *> storeFeed(FeedCore::Subscription *feed) final;
    QFuture<FeedCore::ArticleRef> getByUrl(const QUrl &url) final;
    QFuture<FeedCore::ArticleRef> storeSavedPage(FeedCore::Subscription *feed, const FeedCore::SavedPage &page) final;
    QFuture<FeedCore::ArticleRef> getUnfiled();

    /**
     * The placeholder feed that owns articles which don't belong to any subscription.
     */
    FeedCore::Feed *unfiledFeed();
    void listenForChanges(FeedImpl *feed);
    void expire(FeedImpl *feed, const QDateTime &olderThan);

private:
    class WorkerThread;
    class Worker;
    WorkerThread *m_thread;
    Worker *m_worker;
    FeedCore::Feed *m_unfiledFeed{nullptr};
    void onFeedRequestDelete(FeedImpl *feed);
    void onUpdateIntervalChanged(FeedImpl *feed);
    void onExpireModeChanged(FeedImpl *feed);
    void onExpireAgeChanged(FeedImpl *feed);
    void onUpdateModeChanged(FeedImpl *feed);
};
}
