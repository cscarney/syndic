#include "context.h"
#include "future.h"
#include "provisionalfeed.h"
#include "sqlite/storageimpl.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>

static constexpr const char *testDbName = "testStoreAndRetrieveFeed.db";
static constexpr const char *testFeedName = "testName";
static constexpr const char *testUrl = "about:blank";
static constexpr const int testContextUpdateInterval = 1206;
static constexpr const int testFeedUpdateInterval = 20;
static constexpr const int testContextExpireAge = 1532;
static constexpr const int testFeedExpireAge = 611;

class testStoreAndRetrieveFeed : public QObject
{
    Q_OBJECT

    FeedCore::Context *m_context{nullptr};
    FeedCore::Feed *m_feed{nullptr};

    void refreshContext()
    {
        delete m_context;
        m_context = new FeedCore::Context(new SqliteStorage::StorageImpl(testDbName));
        m_context->setDefaultUpdateInterval(testContextUpdateInterval);
        m_context->setExpireAge(testContextExpireAge);
        QSignalSpy waitForFeeds(m_context, &FeedCore::Context::feedListPopulated);
        waitForFeeds.wait();
        auto feeds = m_context->getFeeds();
        m_feed = feeds.isEmpty() ? nullptr : *feeds.begin();
    }

    void setupModifyUpdateModeTest(FeedCore::Feed::UpdateMode mode)
    {
        m_feed->setUpdateInterval(testFeedUpdateInterval);
        m_feed->setUpdateMode(mode);
        QCoreApplication::processEvents();
        refreshContext();
    }

    void setupModifyExpireModeTest(FeedCore::Feed::UpdateMode mode)
    {
        m_feed->setExpireAge(testFeedExpireAge);
        m_feed->setExpireMode(mode);
        QCoreApplication::processEvents();
        refreshContext();
    }

private slots:
    void initTestCase()
    {
        qRegisterMetaType<FeedCore::Feed *>();
    }

    void init()
    {
        {
            QFile(testDbName).remove();
            refreshContext();
            FeedCore::ProvisionalFeed testFeed;
            testFeed.setUrl(QUrl(testUrl));
            testFeed.setName(testFeedName);
            QSignalSpy waitForFeed(m_context, &FeedCore::Context::feedAdded);
            m_context->addFeed(&testFeed);
            QVERIFY(waitForFeed.count() || waitForFeed.wait());
        }

        refreshContext();
        QVERIFY(m_feed != nullptr);
    }

    void cleanup()
    {
        delete m_context;
        m_context = nullptr;
        QFile(testDbName).remove();
    }

    void testUrlFieldPreserved()
    {
        QVERIFY(m_feed->url() == QUrl(testUrl));
    }

    void testNameFieldPreserved()
    {
        QVERIFY(m_feed->name() == testFeedName);
    }

    void testModifyName()
    {
        {
            m_feed->setName("new name");
            QCoreApplication::processEvents();
        }
        {
            refreshContext();
            QVERIFY(m_feed != nullptr);
            QVERIFY(m_feed->name() == "new name");
        }
    }

    void testSetUpdateModeInherit()
    {
        setupModifyUpdateModeTest(FeedCore::Feed::InheritUpdateMode);
        QVERIFY(m_feed != nullptr);
        QVERIFY(m_feed->updateMode() == FeedCore::Feed::InheritUpdateMode);
        QVERIFY(m_feed->updateInterval() == testContextUpdateInterval);
    }

    void testSetUpdateModeOverride()
    {
        setupModifyUpdateModeTest(FeedCore::Feed::OverrideUpdateMode);
        QVERIFY(m_feed != nullptr);
        QVERIFY(m_feed->updateMode() == FeedCore::Feed::OverrideUpdateMode);
        QVERIFY(m_feed->updateInterval() == testFeedUpdateInterval);
    }

    void testSetUpdateModeDisable()
    {
        setupModifyUpdateModeTest(FeedCore::Feed::DisableUpdateMode);
        QVERIFY(m_feed != nullptr);
        QVERIFY(m_feed->updateMode() == FeedCore::Feed::DisableUpdateMode);
    }

    void testSetExpireModeInherit()
    {
        setupModifyExpireModeTest(FeedCore::Feed::InheritUpdateMode);
        QVERIFY(m_feed != nullptr);
        QVERIFY(m_feed->expireMode() == FeedCore::Feed::InheritUpdateMode);
        QVERIFY(m_feed->expireAge() == testContextExpireAge);
    }

    void testSetExpireModeOverride()
    {
        setupModifyExpireModeTest(FeedCore::Feed::OverrideUpdateMode);
        QVERIFY(m_feed != nullptr);
        QVERIFY(m_feed->expireMode() == FeedCore::Feed::OverrideUpdateMode);
        QVERIFY(m_feed->expireAge() == testFeedExpireAge);
    }

    void testSetExpireModeDisable()
    {
        setupModifyExpireModeTest(FeedCore::Feed::DisableUpdateMode);
        QVERIFY(m_feed->expireMode() == FeedCore::Feed::DisableUpdateMode);
    }
};

QTEST_MAIN(testStoreAndRetrieveFeed)

#include "tst_teststoreandretrievefeed.moc"
