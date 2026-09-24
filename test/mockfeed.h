#pragma once
#include "articleref.h"
#include "feed.h"
#include <QList>

/**
 * A Feed that is not a Subscription, as returned by a backend that manages subscriptions remotely.
 */
class MockFeed : public FeedCore::Feed
{
public:
    int m_updateCount{0};
    int m_cancelCount{0};
    QList<FeedCore::ArticleRef> m_articles;

    QFuture<FeedCore::ArticleRef> getArticles(bool /*unreadFilter*/) override
    {
        return FeedCore::Future::yield<FeedCore::ArticleRef>(this, [this](auto &op) {
            for (const auto &item : std::as_const(m_articles)) {
                op.addResult(item);
            }
        });
    }

    QDateTime m_lastUpdateTimestamp;

    void update(const QDateTime &timestamp = QDateTime::currentDateTime()) override
    {
        m_updateCount++;
        m_lastUpdateTimestamp = timestamp;
    }

    void cancelUpdates() override
    {
        m_cancelCount++;
    }
};
