/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "context.h"
#include "mockarticle.h"
#include "mockstorage.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>
using namespace FeedCore;

class testAllItemsFeed : public QObject
{
    Q_OBJECT
    QSharedPointer<Feed> m_allItemsFeed;
    QPointer<MockStorage> m_mockStorage;
    QScopedPointer<MockFeed> m_mockFeed1;
    QScopedPointer<MockFeed> m_mockFeed2;
    QScopedPointer<FeedCore::Context> m_context;

private slots:
    void initTestCase()
    {
        qRegisterMetaType<FeedCore::ArticleRef>();
    }

    void init()
    {
        m_mockFeed1.reset(new MockFeed);
        m_mockFeed1->m_articles = {QSharedPointer<MockArticle>(new MockArticle(m_mockFeed1.get())),
                                   QSharedPointer<MockArticle>(new MockArticle(m_mockFeed1.get()))};

        m_mockFeed2.reset(new MockFeed);
        m_mockFeed2->m_articles = {QSharedPointer<MockArticle>(new MockArticle(m_mockFeed2.get())),
                                   QSharedPointer<MockArticle>(new MockArticle(m_mockFeed2.get()))};

        m_mockStorage = new MockStorage;
        m_mockStorage->m_feeds = {m_mockFeed1.get(), m_mockFeed2.get()};
        m_context.reset(new Context(m_mockStorage));

        m_allItemsFeed = m_context->allItemsFeed();

        if (!QTest::qWaitFor([this] {
                return m_context->feedListComplete();
            })) {
            qCritical() << "Feed list did not complete";
        }
    }

    void cleanup()
    {
        m_context.reset();
        m_mockFeed1.reset();
        m_mockFeed2.reset();
    }

    void testAllItemsFeedContainsAllItems()
    {
        QList<ArticleRef> allItems;
        allItems << m_mockFeed1->m_articles;
        allItems << m_mockFeed2->m_articles;
        std::sort(allItems.begin(), allItems.end());

        auto getArticles = m_allItemsFeed->getArticles(false);
        QList<ArticleRef> aafItems;
        FeedCore::Future::safeThen(getArticles, this, [&aafItems](auto &getArticles) {
            aafItems = Future::safeResults(getArticles);
        });
        QVERIFY(QTest::qWaitFor([&] {
            return getArticles.isFinished();
        }));
        std::sort(aafItems.begin(), aafItems.end());
        QVERIFY(allItems == aafItems);
    }

    void testAllItemsFeedImplementsUnreadFilter()
    {
        m_mockFeed1->m_articles[0]->setRead(true);
        m_mockFeed2->m_articles[1]->setRead(true);

        QList<ArticleRef> unreadItems = {m_mockFeed1->m_articles[1], m_mockFeed2->m_articles[0]};
        std::sort(unreadItems.begin(), unreadItems.end());

        auto getArticles = m_allItemsFeed->getArticles(true);
        QList<ArticleRef> aafItems;
        FeedCore::Future::safeThen(getArticles, this, [&aafItems](auto &getArticles) {
            aafItems = Future::safeResults(getArticles);
        });
        QVERIFY(QTest::qWaitFor([&] {
            return getArticles.isFinished();
        }));
        std::sort(aafItems.begin(), aafItems.end());
        QVERIFY(unreadItems == aafItems);
    }

    void testAllItemsFeedEmitsArticleAddedSignals()
    {
        QSignalSpy waitArticleAdded(m_allItemsFeed.get(), &FeedCore::Feed::articleAdded);
        ArticleRef art(new MockArticle(m_mockFeed1.get()));
        emit m_mockFeed1->articleAdded(art);
        qDebug() << waitArticleAdded;
        QVERIFY(waitArticleAdded.length() == 1);
    }
};

QTEST_MAIN(testAllItemsFeed)

#include "tst_allitemsfeed.moc"
