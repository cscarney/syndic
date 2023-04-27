#pragma once
#include "articleref.h"
#include "feed.h"
#include <QList>

class MockFeed : public FeedCore::Feed
{
public:
    class Updater : public FeedCore::Feed::Updater
    {
    public:
        int m_call_count{0};
        void run() override
        {
            m_call_count++;
        }
        using Feed::Updater::finish;
        using Feed::Updater::setError;
        using Feed::Updater::Updater;
    };
    Updater m_updater;
    QList<FeedCore::ArticleRef> m_articles;

    Feed::Updater *updater() override
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

    MockFeed()
        : m_updater(this, this)
    {
    }
};
