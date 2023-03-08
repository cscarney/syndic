#pragma once
#include "feed.h"

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

    Feed::Updater *updater() override
    {
        return &m_updater;
    }

    FeedCore::Future<FeedCore::ArticleRef> *getArticles(bool /*unreadFilter*/) override
    {
        Q_UNREACHABLE();
    }

    MockFeed()
        : m_updater(this, this)
    {
    }
};
