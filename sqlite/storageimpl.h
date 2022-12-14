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
    FeedCore::Future<FeedCore::ArticleRef> *getById(qint64 id);
    FeedCore::Future<FeedCore::ArticleRef> *getByFeed(FeedImpl *feedId);
    FeedCore::Future<FeedCore::ArticleRef> *getUnreadByFeed(FeedImpl *feedId);
    FeedCore::Future<FeedCore::ArticleRef> *storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item);
    FeedCore::Future<QString> *getContent(ArticleImpl *article);
    FeedCore::Future<QString> *getReadableContent(ArticleImpl *article);
    void cacheReadableContent(ArticleImpl *article, const QString &readableContent);
    void onArticleReadChanged(ArticleImpl *article);
    void onArticleStarredChanged(ArticleImpl *article);

    FeedCore::Future<FeedCore::ArticleRef> *getAll() final;
    FeedCore::Future<FeedCore::ArticleRef> *getUnread() final;
    FeedCore::Future<FeedCore::ArticleRef> *getStarred() final;
    FeedCore::Future<FeedCore::Feed *> *getFeeds() final;
    FeedCore::Future<FeedCore::Feed *> *storeFeed(FeedCore::Feed *feed) final;
    void listenForChanges(FeedImpl *feed);
    void expire(FeedImpl *feed, const QDateTime &olderThan);

protected:
    void customEvent(QEvent *e) override;

private:
    FeedDatabase m_db;
    FeedCore::ObjectFactory<qint64, FeedImpl> m_feedFactory;
    FeedCore::SharedFactory<qint64, ArticleImpl> m_articleFactory;
    bool m_hasTransaction{false};
    void appendFeedResults(FeedCore::Future<FeedCore::Feed *> *op, FeedQuery &q);
    void appendArticleResults(FeedCore::Future<FeedCore::ArticleRef> *op, ItemQuery &q);
    void onFeedRequestDelete(FeedImpl *feed);
    void ensureTransaction();
    const static int CommitEvent;

    template<typename Payload, typename Func>
    FeedCore::Future<Payload> *runInTransaction(Func func)
    {
        ensureTransaction();
        return FeedCore::Future<Payload>::yield(this, func);
    }
};
}
