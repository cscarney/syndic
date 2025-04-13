/**
 * SPDX-FileCopyrightText: 2023 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "automation/automationengine.h"
#include "automation/automationrule.h"
#include "context.h"
#include "mockarticle.h"
#include "mockstorage.h"
#include <QCoreApplication>
#include <QFuture>
#include <QSignalSpy>
#include <QThread>
#include <QtTest>

using namespace FeedCore;

class TestAutomationEngine : public QObject
{
    Q_OBJECT
    QScopedPointer<FeedCore::Context> m_context;
    QPointer<MockStorage> m_mockStorage;
    QScopedPointer<MockFeed> m_mockFeed;
    QScopedPointer<AutomationEngine> m_engine;

    class TestRule : public AbstractAutomationRule
    {
    public:
        int actionCount{0};
        int matchCount{0};
        bool shouldMatch{true};
        int actionDelayMs{0};

        explicit TestRule(QObject *parent = nullptr)
            : AbstractAutomationRule(parent)
        {
        }

        bool matches(const ArticleRef &article) override
        {
            Q_UNUSED(article);
            matchCount += 1;
            return shouldMatch;
        }

        QFuture<void> beginPerformAction(const ArticleRef &article) override
        {
            Q_UNUSED(article);
            actionCount += 1;
            if (actionDelayMs > 0) {
                // create a QFuture that will complete after actionDelayMs milliseconds
                auto *promise = new QPromise<void>();
                promise->start();
                QTimer::singleShot(actionDelayMs, [promise]() {
                    promise->finish();
                    delete promise;
                });
                return promise->future();
            }
            return QtFuture::makeReadyVoidFuture();
        }
    };

private slots:
    void initTestCase()
    {
        qRegisterMetaType<FeedCore::ArticleRef>();
    }

    void init()
    {
        m_mockFeed.reset(new MockFeed);
        m_mockStorage = new MockStorage;
        m_mockStorage->m_feeds = {m_mockFeed.get()};
        m_context.reset(new Context(m_mockStorage));

        if (!QTest::qWaitFor([this] {
                return m_context->feedListComplete();
            })) {
            qCritical() << "Feed list did not complete";
        }

        m_engine.reset(new AutomationEngine(m_context.get()));
    }

    void cleanup()
    {
        m_engine.reset();
        m_context.reset();
        m_mockFeed.reset();
    }

    void testRuleTriggersOnlyWhenMatching()
    {
        auto *matchingRule = new TestRule(m_engine.get());
        matchingRule->shouldMatch = true;
        m_engine->addAutomationRule(matchingRule);

        auto *nonMatchingRule = new TestRule(m_engine.get());
        nonMatchingRule->shouldMatch = false;
        m_engine->addAutomationRule(nonMatchingRule);

        ArticleRef article(new MockArticle(m_mockFeed.get()));
        m_engine->processArticle(article);

        // Wait for all actions to complete
        QVERIFY(QTest::qWaitFor([&] {
            return !m_engine->active();
        }));

        // Both rules should have been checked for matches
        QCOMPARE(matchingRule->matchCount, 1);
        QCOMPARE(nonMatchingRule->matchCount, 1);

        // Only the matching rule should have performed an action
        QCOMPARE(matchingRule->actionCount, 1);
        QCOMPARE(nonMatchingRule->actionCount, 0);
    }

    void testQueueLimitRespected()
    {
        const int ruleCount = QThread::idealThreadCount() * 2;
        const int expectedMaxConcurrent = QThread::idealThreadCount();
        const int delayMs = 200; // Add a delay to ensure actions don't complete too quickly

        // Create a set of rules that will all match
        QList<TestRule *> rules;
        for (int i = 0; i < ruleCount; ++i) {
            auto *rule = new TestRule(m_engine.get());
            rule->shouldMatch = true;
            rule->actionDelayMs = delayMs;
            m_engine->addAutomationRule(rule);
            rules.append(rule);
        }

        // Process an article, which should trigger all rules
        ArticleRef article(new MockArticle(m_mockFeed.get()));
        m_engine->processArticle(article);

        QTest::qWait(50); // Short wait to allow initial actions to start

        // Initially, only idealThreadCount rules should be running actions
        int initialActionCount = 0;
        for (auto *rule : rules) {
            initialActionCount += rule->actionCount;
        }
        QCOMPARE(initialActionCount, expectedMaxConcurrent);

        // Eventually all rules should run
        QVERIFY(QTest::qWaitFor(
            [&] {
                int totalActionCount = 0;
                for (auto *rule : rules) {
                    totalActionCount += rule->actionCount;
                }
                return totalActionCount == ruleCount;
            },
            delayMs * 3));
    }
};

QTEST_MAIN(TestAutomationEngine)

#include "tst_automationengine.moc"
