#pragma once
#include "articleref.h"
#include "feed.h"
#include <QVector>

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
    QVector<FeedCore::ArticleRef> m_articles;

    Feed::Updater *updater() override
    {
        return &m_updater;
    }

    FeedCore::Future<FeedCore::ArticleRef> *getArticles(bool /*unreadFilter*/) override
    {
        return FeedCore::Future<FeedCore::ArticleRef>::yield(this, [this](auto *op) {
            op->setResult(QVector<FeedCore::ArticleRef>(m_articles));
        });
    }

    MockFeed()
        : m_updater(this, this)
    {
    }
};
