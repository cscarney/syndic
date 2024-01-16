#pragma once
#include "article.h"
#include "mockfeed.h"
#include "storage.h"

class MockStorage : public FeedCore::Storage
{
public:
    QList<MockFeed *> m_feeds;

    QFuture<FeedCore::ArticleRef> getAll() override
    {
        return FeedCore::Future::yield<FeedCore::ArticleRef>(this, [this](auto &op) {
            for (auto *feed : m_feeds) {
                for (auto &ar : std::as_const(feed->m_articles)) {
                    op.addResult(ar);
                }
            }
        });
    }

    QFuture<FeedCore::ArticleRef> getUnread() override
    {
        return FeedCore::Future::yield<FeedCore::ArticleRef>(this, [this](auto &op) {
            for (auto *feed : m_feeds) {
                for (auto &ar : std::as_const(feed->m_articles)) {
                    if (!ar->isRead()) {
                        op.addResult(ar);
                    }
                }
            }
        });
    }

    QFuture<FeedCore::ArticleRef> getStarred() override
    {
        return FeedCore::Future::yield<FeedCore::ArticleRef>(this, [this](auto &op) {
            for (auto *feed : m_feeds) {
                for (auto &ar : std::as_const(feed->m_articles)) {
                    if (!ar->isStarred()) {
                        op.addResult(ar);
                    }
                }
            }
        });
    }

    QFuture<FeedCore::Feed *> getFeeds() override
    {
        return FeedCore::Future::yield<FeedCore::Feed *>(this, [this](auto &op) {
            for (auto &item : std::as_const(m_feeds)) {
                op.addResult(item);
            }
        });
    }

    QFuture<FeedCore::Feed *> storeFeed(FeedCore::Feed *feed) override
    {
        MockFeed *newFeed = new MockFeed;
        newFeed->setParent(this);
        newFeed->updateParams(feed);
        m_feeds << newFeed;
        return FeedCore::Future::yield<FeedCore::Feed *>(newFeed, [newFeed](auto &op) {
            op.addResult(newFeed);
        });
    }

    QFuture<FeedCore::ArticleRef> getSearchResults(const QString &search) override
    {
        return FeedCore::Future::yield<FeedCore::ArticleRef>(this, [this, search](auto &op) {
            for (auto *feed : m_feeds) {
                for (auto &ar : std::as_const(feed->m_articles)) {
                    if (ar->title().contains(search)) {
                        op.addResult(ar);
                    }
                }
            }
        });
    }

    QFuture<FeedCore::ArticleRef> getHighlights(size_t limit) override
    {
        return getAll();
    }
};
