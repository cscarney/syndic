#include "article.h"
#include "context.h"
#include "future.h"
#include "provisionalfeed.h"
#include "sqlite/storageimpl.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>

#include "atomFeedTemplate.h"

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

    QUrl writeAtomFeedTestXml(const QDateTime &date1, const QDateTime &date2)
    {
        constexpr const char *filename = "testatom.xml";
        QString content = QString(testAtomFeedTemplate).arg(date1.toString(Qt::ISODate)).arg(date2.toString(Qt::ISODate));
        QString absolutePath = QFileInfo(filename).absoluteFilePath();
        QFile file(absolutePath);
        if (!file.open(QIODevice::WriteOnly)) {
            assert(!"Couldn't open testatom.xml");
        }
        file.write(content.toUtf8());
        file.close();
        return QUrl::fromLocalFile(absolutePath);
    }

    QVector<FeedCore::ArticleRef> getArticles(FeedCore::Feed *feed)
    {
        auto *articlesFuture = feed->getArticles(true);
        QVector<FeedCore::ArticleRef> articles;
        QObject::connect(articlesFuture, &FeedCore::BaseFuture::finished, [articlesFuture, &articles]() {
            qDebug() << "got articles" << articlesFuture->result();
            articles = articlesFuture->result();
        });
        QSignalSpy(articlesFuture, &FeedCore::BaseFuture::finished).wait();
        return articles;
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

    void testModifyUrl()
    {
        const QUrl testUrl = QUrl("http://new.url");
        {
            m_feed->setUrl(testUrl);
            QCoreApplication::processEvents();
        }
        {
            refreshContext();
            QVERIFY(m_feed != nullptr);
            QVERIFY(m_feed->url() == testUrl);
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

    void testStoreArticles()
    {
        {
            QCoreApplication::processEvents();
            QUrl feedUrl = writeAtomFeedTestXml(QDateTime::currentDateTime(), QDateTime::currentDateTime());
            m_feed->setUrl(feedUrl);
            m_feed->updater()->start();
            QSignalSpy(m_feed, &FeedCore::Feed::statusChanged).wait();
            QCoreApplication::processEvents();
        }
        {
            refreshContext();
            auto articles = getArticles(m_feed);
            QVERIFY(articles.length() == 2);
        }
    }

    static bool dateCompare(const QDateTime &d1, const QDateTime &d2)
    {
        return (d1.toSecsSinceEpoch() == d2.toSecsSinceEpoch());
    }

    void testExistingArticlesUpdatedWhenNewContentStored()
    {
        {
            QCoreApplication::processEvents();
            QUrl feedUrl = writeAtomFeedTestXml(QDateTime::currentDateTime().addSecs(-120), QDateTime::currentDateTime().addSecs(-120));
            m_feed->setUrl(feedUrl);
            m_feed->updater()->start();
            QSignalSpy(m_feed, &FeedCore::Feed::statusChanged).wait();
            QCoreApplication::processEvents();
        }
        {
            refreshContext();
            auto articles = getArticles(m_feed);
            QVERIFY(articles.length() == 2);

            QDateTime newDateTime = QDateTime::currentDateTime();

            QUrl feedUrl = writeAtomFeedTestXml(newDateTime, newDateTime);
            m_feed->setUrl(feedUrl);
            m_feed->updater()->start();
            QSignalSpy(articles.at(0).get(), &FeedCore::Article::dateChanged).wait();

            QVERIFY(dateCompare(articles.at(0)->date(), newDateTime));
            QVERIFY(dateCompare(articles.at(1)->date(), newDateTime));
        }
    }
};

QTEST_MAIN(testStoreAndRetrieveFeed)

#include "tst_teststoreandretrievefeed.moc"
