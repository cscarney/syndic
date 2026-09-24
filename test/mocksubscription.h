#pragma once
#include "articleref.h"
#include "subscription.h"
#include <QList>

class MockSubscription : public FeedCore::Subscription
{
public:
    class Updater : public FeedCore::Subscription::Updater
    {
    public:
        int m_call_count{0};
        void run() override
        {
            m_call_count++;
        }
        using Subscription::Updater::finish;
        using Subscription::Updater::setError;
        using Subscription::Updater::Updater;
    };
    Updater m_updater;
    QList<FeedCore::ArticleRef> m_articles;

    Subscription::Updater *updater() override
    {
        return &m_updater;
    }

    QFuture<FeedCore::ArticleRef> getArticles(bool /*unreadFilter*/) override
    {
        return FeedCore::Future::yield<FeedCore::ArticleRef>(this, [this](auto &op) {
            for (const auto &item : std::as_const(m_articles)) {
                op.addResult(item);
            }
        });
    }

    MockSubscription()
        : m_updater(this, this)
    {
    }
};
