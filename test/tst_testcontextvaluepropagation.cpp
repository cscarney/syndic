/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "context.h"
#include "future.h"
#include "provisionalfeed.h"
#include "storage.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>
using namespace FeedCore;

class MockStorage : public FeedCore::Storage
{
    QVector<Feed *> m_feeds;

public:
    QFuture<FeedCore::ArticleRef> getAll() override
    {
        return Future::yield<ArticleRef>(this, [](auto &) {});
    }

    QFuture<FeedCore::ArticleRef> getUnread() override
    {
        return Future::yield<ArticleRef>(this, [](auto &) {});
    }
    QFuture<FeedCore::ArticleRef> getStarred() override
    {
        return Future::yield<ArticleRef>(this, [](auto &) {});
    }
    QFuture<FeedCore::Feed *> getFeeds() override
    {
        return Future::yield<Feed *>(this, [this](auto &r) {
            for (const auto &item : std::as_const(m_feeds)) {
                r.addResult(item);
            }
        });
    }
    QFuture<FeedCore::Feed *> storeFeed(FeedCore::Feed *feed) override
    {
        m_feeds.append(feed);
        return Future::yield<Feed *>(this, [feed](auto &r) {
            r.addResult(feed);
        });
    }
};

constexpr const int contextUpdateInterval = 1904;
constexpr const int contextExpireAge = 4474;

class testContextValuePropagation : public QObject
{
    Q_OBJECT

    FeedCore::Context *m_context{nullptr};

private slots:
    void initTestCase()
    {
        qRegisterMetaType<FeedCore::Feed *>();
    }

    void init()
    {
        m_context = new Context(new MockStorage);
        m_context->setDefaultUpdateInterval(contextUpdateInterval);
        m_context->setExpireAge(contextExpireAge);
    }

    void cleanup()
    {
        delete m_context;
    }

    void testContextPropagatesUpdateInterval()
    {
        const int feedUpdateInterval = 11304;
        ProvisionalFeed feedWithInheritUpdateMode;
        feedWithInheritUpdateMode.setUpdateMode(Feed::InheritUpdateMode);
        feedWithInheritUpdateMode.setUpdateInterval(feedUpdateInterval);

        ProvisionalFeed feedWithOverrideUpdateMode;
        feedWithOverrideUpdateMode.setUpdateMode(Feed::OverrideUpdateMode);
        feedWithOverrideUpdateMode.setUpdateInterval(feedUpdateInterval);

        {
            m_context->addFeed(&feedWithInheritUpdateMode);
            QSignalSpy waitForSignal(&feedWithInheritUpdateMode, &ProvisionalFeed::targetFeedChanged);
            waitForSignal.wait();
        }

        {
            m_context->addFeed(&feedWithOverrideUpdateMode);
            QSignalSpy waitForSignal(&feedWithOverrideUpdateMode, &ProvisionalFeed::targetFeedChanged);
            waitForSignal.wait();
        }

        QVERIFY(feedWithInheritUpdateMode.updateInterval() == contextUpdateInterval);
        QVERIFY(feedWithOverrideUpdateMode.updateInterval() == feedUpdateInterval);

        feedWithOverrideUpdateMode.setUpdateMode(Feed::InheritUpdateMode);
        QVERIFY(feedWithOverrideUpdateMode.updateInterval() == contextUpdateInterval);
    }

    void testContextPropagatesExpireAge()
    {
        const int feedExpireAge = 9933;
        ProvisionalFeed feedWithInheritExpireMode;
        feedWithInheritExpireMode.setExpireMode(Feed::InheritUpdateMode);
        feedWithInheritExpireMode.setExpireAge(feedExpireAge);

        ProvisionalFeed feedWithOverrideExpireMode;
        feedWithOverrideExpireMode.setExpireMode(Feed::OverrideUpdateMode);
        feedWithOverrideExpireMode.setExpireAge(feedExpireAge);

        QSignalSpy waitForSignal(m_context, &Context::feedAdded);
        m_context->addFeed(&feedWithInheritExpireMode);
        waitForSignal.wait();

        m_context->addFeed(&feedWithOverrideExpireMode);
        waitForSignal.wait();

        QVERIFY(feedWithInheritExpireMode.expireAge() == contextExpireAge);
        QVERIFY(feedWithOverrideExpireMode.expireAge() == feedExpireAge);

        feedWithOverrideExpireMode.setExpireMode(Feed::InheritUpdateMode);
        QVERIFY(feedWithOverrideExpireMode.expireAge() == contextExpireAge);
    }
};

QTEST_MAIN(testContextValuePropagation)

#include "tst_testcontextvaluepropagation.moc"
