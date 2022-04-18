#include "feed.h"
#include "scheduler.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>

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

class testUpdateScheduler : public QObject
{
    Q_OBJECT
    FeedCore::Scheduler *scheduler{nullptr};
private slots:
    void init()
    {
        scheduler = new FeedCore::Scheduler();
    }

    void cleanup()
    {
        delete scheduler;
    }

    void testStaleFeedGetsUpdatedWhenScheduled()
    {
        MockFeed staleFeed;
        const QDateTime lastUpdate = QDateTime::currentDateTime().addSecs(-10);
        const int updateInterval = 5;
        staleFeed.setLastUpdate(lastUpdate);
        staleFeed.setUpdateInterval(updateInterval);
        QDateTime timestamp = QDateTime::currentDateTime();
        scheduler->schedule(&staleFeed, timestamp);
        QVERIFY(staleFeed.status() == FeedCore::Feed::Updating);
        staleFeed.m_updater.finish();
        QVERIFY(staleFeed.lastUpdate() == timestamp);
    }

    void testFeedGetsUpdatedWhenScheduledTimeArrives()
    {
        MockFeed notQuiteStaleFeed;
        const QDateTime lastUpdate = QDateTime::currentDateTime().addSecs(-30);
        const int updateInterval = 32;
        notQuiteStaleFeed.setLastUpdate(lastUpdate);
        notQuiteStaleFeed.setUpdateInterval(updateInterval);
        scheduler->schedule(&notQuiteStaleFeed);
        scheduler->start(1);
        QVERIFY(notQuiteStaleFeed.status() == FeedCore::Feed::Idle);
        QSignalSpy waitForStatusChange(&notQuiteStaleFeed, &FeedCore::Feed::statusChanged);
        bool gotSignal = waitForStatusChange.wait();
        QVERIFY(gotSignal);
        QVERIFY(notQuiteStaleFeed.status() == FeedCore::Feed::Updating);
        notQuiteStaleFeed.m_updater.finish();
        QVERIFY(notQuiteStaleFeed.status() == FeedCore::Feed::Idle);
        QVERIFY(notQuiteStaleFeed.lastUpdate() >= lastUpdate.addSecs(updateInterval));
    }

    void testFeedGetsRescheduledWhenUpdateIntervalChanges()
    {
        const QDateTime lastUpdate = QDateTime::currentDateTime().addSecs(-30);
        const int shortUpdateInterval = 32;
        const int longUpdateInterval = 40;
        const int longerUpdateInterval = 50;

        MockFeed feed1;
        feed1.setLastUpdate(lastUpdate);
        feed1.setUpdateInterval(longUpdateInterval);
        scheduler->schedule(&feed1);

        MockFeed feed2;
        feed2.setLastUpdate(lastUpdate);
        feed2.setUpdateInterval(longerUpdateInterval);
        scheduler->schedule(&feed2);
        scheduler->start(1);

        QVERIFY(feed1.status() == FeedCore::Feed::Idle);
        QVERIFY(feed2.status() == FeedCore::Feed::Idle);

        feed2.setUpdateInterval(shortUpdateInterval);
        QSignalSpy waitForStatusChange(&feed2, &FeedCore::Feed::statusChanged);
        bool gotSignal = waitForStatusChange.wait();
        QVERIFY(gotSignal);
        QVERIFY(feed2.status() == FeedCore::Feed::Updating);

        feed2.m_updater.finish();
        QVERIFY(feed2.status() == FeedCore::Feed::Idle);
    }

    void testFeedNotScheduledWhileBeingUpdated()
    {
        MockFeed feedWithShortUpdateInterval;
        const QDateTime lastUpdate = QDateTime::currentDateTime();
        feedWithShortUpdateInterval.setLastUpdate(lastUpdate);
        feedWithShortUpdateInterval.setUpdateInterval(1);
        scheduler->schedule(&feedWithShortUpdateInterval);
        scheduler->start(1);

        QSignalSpy waitForStatusChange(&feedWithShortUpdateInterval, &FeedCore::Feed::statusChanged);
        bool gotFirstSignal = waitForStatusChange.wait();
        QVERIFY(gotFirstSignal);
        QVERIFY(feedWithShortUpdateInterval.status() == FeedCore::Feed::Updating);

        const int waitTime = 2000;
        bool gotSecondSignal = waitForStatusChange.wait(waitTime);
        QVERIFY(!gotSecondSignal);
        feedWithShortUpdateInterval.m_updater.finish();

        QVERIFY(feedWithShortUpdateInterval.m_updater.m_call_count == 1);
    }

    void testAddFeedDuringUpdate()
    {
        MockFeed feed;
        const QDateTime lastUpdate = QDateTime::currentDateTime().addSecs(-10);
        feed.setLastUpdate(lastUpdate);
        feed.setUpdateInterval(1);
        const QDateTime startTime = QDateTime::currentDateTime().addSecs(-2);
        feed.m_updater.start(startTime);
        scheduler->schedule(&feed, QDateTime::currentDateTime());
        QVERIFY(feed.m_updater.updateStartTime() == startTime);
    }

    void testFeedNotScheduledAfterBeingRemoved()
    {
        const QDateTime lastUpdate = QDateTime::currentDateTime();
        MockFeed feed;
        feed.setLastUpdate(lastUpdate);
        feed.setUpdateInterval(1);
        scheduler->schedule(&feed);
        scheduler->start(1);
        feed.m_updater.start();
        scheduler->unschedule(&feed);
        feed.m_updater.finish();
        const int waitTime = 2000;
        QSignalSpy waitForStatusChange(&feed, &FeedCore::Feed::statusChanged);
        bool gotSignal = waitForStatusChange.wait(waitTime);
        QVERIFY(!gotSignal);
        QVERIFY(feed.m_updater.m_call_count == 1);
    }

    void testFeedScheduledAfterBeingUpdated()
    {
        const QDateTime timestamp = QDateTime::currentDateTime();
        const QDateTime lastUpdate = timestamp.addSecs(1);
        MockFeed feed;
        feed.setLastUpdate(lastUpdate);
        feed.setUpdateInterval(1);
        scheduler->schedule(&feed);
        scheduler->start(1);
        feed.m_updater.start(timestamp);
        feed.m_updater.finish();
        QVERIFY(feed.lastUpdate() == timestamp);

        QSignalSpy waitForStatusChange(&feed, &FeedCore::Feed::statusChanged);
        const int waitTime = 2000;
        bool gotSignal = waitForStatusChange.wait(waitTime);
        QVERIFY(gotSignal);
        QVERIFY(feed.status() == FeedCore::Feed::Updating);
        QVERIFY(feed.m_updater.m_call_count == 2);
        feed.m_updater.finish();
        QVERIFY(feed.lastUpdate() >= lastUpdate);
    }

    void testFeedScheduledAfterError()
    {
        const QDateTime timestamp = QDateTime::currentDateTime();
        const QDateTime lastUpdate = timestamp.addSecs(-10);
        MockFeed feed;
        feed.setLastUpdate(lastUpdate);
        feed.setUpdateInterval(1);
        scheduler->schedule(&feed);
        scheduler->start(1);
        QVERIFY(feed.status() == FeedCore::Feed::Updating);
        feed.m_updater.setError("error");
        QVERIFY(feed.status() == FeedCore::Feed::Error);

        QSignalSpy waitForStatusChange(&feed, &FeedCore::Feed::statusChanged);
        bool gotSignal = waitForStatusChange.wait();
        QVERIFY(gotSignal);
        QVERIFY(feed.status() == FeedCore::Feed::Updating);
        QVERIFY(feed.m_updater.m_call_count == 2);
    }

    void testErrorFeedUpdatesWhenErrorsCleared()
    {
        const QDateTime timestamp = QDateTime::currentDateTime();
        const QDateTime lastUpdate = timestamp.addSecs(-10);
        MockFeed feed;
        feed.setLastUpdate(lastUpdate);
        feed.setUpdateInterval(1);
        scheduler->schedule(&feed);
        scheduler->start(1);
        QVERIFY(feed.status() == FeedCore::Feed::Updating);
        feed.m_updater.setError("error");
        QVERIFY(feed.status() == FeedCore::Feed::Error);

        scheduler->clearErrors();
        QVERIFY(feed.status() == FeedCore::Feed::Updating);
        QVERIFY(feed.m_updater.m_call_count == 2);
    }

    void testFeedsWithSameUpdateIntervalAreUpdatedTogether()
    {
        const QDateTime timestamp = QDateTime::currentDateTime();
        const int updateInterval = 2;

        MockFeed feed1;
        const QDateTime lastUpdate1 = timestamp.addSecs(-3);
        feed1.setLastUpdate(lastUpdate1);
        feed1.setUpdateInterval(updateInterval);

        MockFeed feed2;
        const QDateTime lastUpdate2 = timestamp.addSecs(-4);
        feed1.setLastUpdate(lastUpdate2);
        feed1.setUpdateInterval(updateInterval);

        scheduler->schedule(&feed1, timestamp);
        scheduler->schedule(&feed2, timestamp);

        QVERIFY(feed1.status() == FeedCore::Feed::Updating);
        QVERIFY(feed2.status() == FeedCore::Feed::Updating);

        feed1.m_updater.finish();
        feed2.m_updater.finish();

        QVERIFY(feed1.lastUpdate() == feed2.lastUpdate());
    }
};

QTEST_MAIN(testUpdateScheduler)

#include "tst_testupdatescheduler.moc"
