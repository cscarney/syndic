#pragma once
#include "article.h"
#include "mockfeed.h"
#include "storage.h"

class MockStorage : public FeedCore::Storage
{
public:
    QVector<MockFeed *> m_feeds;

    FeedCore::Future<FeedCore::ArticleRef> *getAll() override
    {
        return FeedCore::Future<FeedCore::ArticleRef>::yield(this, [this](auto *op) {
            for (auto *feed : m_feeds) {
                for (auto &ar : std::as_const(feed->m_articles)) {
                    op->appendResult(ar);
                }
            }
        });
    }

    FeedCore::Future<FeedCore::ArticleRef> *getUnread() override
    {
        return FeedCore::Future<FeedCore::ArticleRef>::yield(this, [this](auto *op) {
            for (auto *feed : m_feeds) {
                for (auto &ar : std::as_const(feed->m_articles)) {
                    if (!ar->isRead()) {
                        op->appendResult(ar);
                    }
                }
            }
        });
    }

    FeedCore::Future<FeedCore::ArticleRef> *getStarred() override
    {
        return FeedCore::Future<FeedCore::ArticleRef>::yield(this, [this](auto *op) {
            for (auto *feed : m_feeds) {
                for (auto &ar : std::as_const(feed->m_articles)) {
                    if (!ar->isStarred()) {
                        op->appendResult(ar);
                    }
                }
            }
        });
    }

    FeedCore::Future<FeedCore::Feed *> *getFeeds() override
    {
        return FeedCore::Future<FeedCore::Feed *>::yield(this, [this](auto *op) {
            op->setResult(QVector<FeedCore::Feed *>(m_feeds.cbegin(), m_feeds.cend()));
        });
    }

    FeedCore::Future<FeedCore::Feed *> *storeFeed(FeedCore::Feed *feed) override
    {
        MockFeed *newFeed = new MockFeed;
        newFeed->setParent(this);
        newFeed->updateParams(feed);
        m_feeds << newFeed;
        return FeedCore::Future<FeedCore::Feed *>::yield(newFeed, [newFeed](auto *op) {
            op->setResult(newFeed);
        });
    }
};
