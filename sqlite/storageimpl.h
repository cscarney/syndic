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
    QFuture<FeedCore::Feed *> getFeeds() final;
    QFuture<FeedCore::Feed *> storeFeed(FeedCore::Feed *feed) final;
    void listenForChanges(FeedImpl *feed);
    void expire(FeedImpl *feed, const QDateTime &olderThan);

protected:
    void customEvent(QEvent *e) override;

private:
    FeedDatabase m_db;
    FeedCore::ObjectFactory<qint64, FeedImpl> m_feedFactory;
    FeedCore::SharedFactory<qint64, ArticleImpl> m_articleFactory;
    bool m_hasTransaction{false};
    void appendFeedResults(QPromise<FeedCore::Feed *> &op, FeedQuery &q);
    void appendArticleResults(QPromise<FeedCore::ArticleRef> &op, ItemQuery &q);
    void onFeedRequestDelete(FeedImpl *feed);
    void ensureTransaction();
    const static int CommitEvent;

    template<typename Payload, typename Func>
    QFuture<Payload> runInTransaction(Func func)
    {
        ensureTransaction();
        return FeedCore::Future::yield<Payload>(this, func);
    }
};
}
