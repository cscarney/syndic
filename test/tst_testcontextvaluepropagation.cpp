/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "context.h"
#include "future.h"
#include "provisionalsubscription.h"
#include "storage.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>
using namespace FeedCore;

class MockStorage : public FeedCore::Storage
{
    QList<Subscription *> m_feeds;

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
    QFuture<FeedCore::Subscription *> getFeeds() override
    {
        return Future::yield<Subscription *>(this, [this](auto &r) {
            for (const auto &item : std::as_const(m_feeds)) {
                r.addResult(item);
            }
        });
    }
    QFuture<FeedCore::Subscription *> storeFeed(FeedCore::Subscription *feed) override
    {
        m_feeds.append(feed);
        return Future::yield<Subscription *>(this, [feed](auto &r) {
            r.addResult(feed);
        });
    }

    QFuture<ArticleRef> getSearchResults(const QString &search) override
    {
        return Future::yield<ArticleRef>(this, [](auto &) {});
    }

    QFuture<ArticleRef> getHighlights(size_t limit) override
    {
        return Future::yield<ArticleRef>(this, [](auto &) {});
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
        qRegisterMetaType<FeedCore::Subscription *>();
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
        ProvisionalSubscription feedWithInheritUpdateMode;
        feedWithInheritUpdateMode.setUpdateMode(Subscription::InheritUpdateMode);
        feedWithInheritUpdateMode.setUpdateInterval(feedUpdateInterval);

        ProvisionalSubscription feedWithOverrideUpdateMode;
        feedWithOverrideUpdateMode.setUpdateMode(Subscription::OverrideUpdateMode);
        feedWithOverrideUpdateMode.setUpdateInterval(feedUpdateInterval);

        {
            m_context->addFeed(&feedWithInheritUpdateMode);
            QSignalSpy waitForSignal(&feedWithInheritUpdateMode, &ProvisionalSubscription::targetFeedChanged);
            waitForSignal.wait();
        }

        {
            m_context->addFeed(&feedWithOverrideUpdateMode);
            QSignalSpy waitForSignal(&feedWithOverrideUpdateMode, &ProvisionalSubscription::targetFeedChanged);
            waitForSignal.wait();
        }

        QVERIFY(feedWithInheritUpdateMode.updateInterval() == contextUpdateInterval);
        QVERIFY(feedWithOverrideUpdateMode.updateInterval() == feedUpdateInterval);

        feedWithOverrideUpdateMode.setUpdateMode(Subscription::InheritUpdateMode);
        QVERIFY(feedWithOverrideUpdateMode.updateInterval() == contextUpdateInterval);
    }

    void testContextPropagatesExpireAge()
    {
        const int feedExpireAge = 9933;
        ProvisionalSubscription feedWithInheritExpireMode;
        feedWithInheritExpireMode.setExpireMode(Subscription::InheritUpdateMode);
        feedWithInheritExpireMode.setExpireAge(feedExpireAge);

        ProvisionalSubscription feedWithOverrideExpireMode;
        feedWithOverrideExpireMode.setExpireMode(Subscription::OverrideUpdateMode);
        feedWithOverrideExpireMode.setExpireAge(feedExpireAge);

        QSignalSpy waitForSignal(m_context, &Context::feedAdded);
        m_context->addFeed(&feedWithInheritExpireMode);
        waitForSignal.wait();

        m_context->addFeed(&feedWithOverrideExpireMode);
        waitForSignal.wait();

        QVERIFY(feedWithInheritExpireMode.expireAge() == contextExpireAge);
        QVERIFY(feedWithOverrideExpireMode.expireAge() == feedExpireAge);

        feedWithOverrideExpireMode.setExpireMode(Subscription::InheritUpdateMode);
        QVERIFY(feedWithOverrideExpireMode.expireAge() == contextExpireAge);
    }
};

QTEST_MAIN(testContextValuePropagation)

#include "tst_testcontextvaluepropagation.moc"
