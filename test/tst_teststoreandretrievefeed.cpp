#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include "future.h"
#include "provisionalfeed.h"
#include "sqlite/storageimpl.h"


static constexpr const char* testDbName = "testStoreAndRetrieveFeed.db";
static constexpr const char* testFeedName = "testName";
static constexpr const char* testUrl = "about:blank";

class testStoreAndRetrieveFeed : public QObject
{
    Q_OBJECT
private slots:
    void init()
    {
        QFile(testDbName).remove();
        Sqlite::StorageImpl storage(testDbName);
        FeedCore::ProvisionalFeed testFeed;
        testFeed.setUrl(QUrl(testUrl));
        testFeed.setName(testFeedName);
        auto *storeFuture = storage.storeFeed(&testFeed);
        FeedCore::Feed *feed { nullptr };
        QObject::connect(storeFuture, &FeedCore::BaseFuture::finished, this, [&feed, storeFuture] {
            QVERIFY(storeFuture->result().count() == 1);
            feed = storeFuture->result()[0];
        });
        QSignalSpy waitForInitialFeed(storeFuture, &FeedCore::BaseFuture::finished);
        waitForInitialFeed.wait();
        QVERIFY(feed!=nullptr);
    }

    void cleanup()
    {
        QFile(testDbName).remove();
    }

    void testUrlFieldPreserved()
    {
        {
            Sqlite::StorageImpl storage(testDbName);
            auto *retrieveFuture = storage.getFeeds();
            FeedCore::Feed *feed { nullptr };
            QObject::connect(retrieveFuture, &FeedCore::BaseFuture::finished, this, [&feed, retrieveFuture]{
                feed = retrieveFuture->result()[0];
            });
            QSignalSpy waitForRetrievedFeed(retrieveFuture, &FeedCore::BaseFuture::finished);
            waitForRetrievedFeed.wait();
            QVERIFY(feed->url() == QUrl(testUrl));
        }
    }

    void testNameFieldPreserved()
    {
        Sqlite::StorageImpl storage(testDbName);
        auto *retrieveFuture = storage.getFeeds();
        FeedCore::Feed *feed { nullptr };
        QObject::connect(retrieveFuture, &FeedCore::BaseFuture::finished, this, [&feed, retrieveFuture]{
            feed = retrieveFuture->result()[0];
        });
        QSignalSpy waitForRetrievedFeed(retrieveFuture, &FeedCore::BaseFuture::finished);
        waitForRetrievedFeed.wait();
        QVERIFY(feed->name() == testFeedName);
    }

    void testModifyName() {
        {
            Sqlite::StorageImpl storage(testDbName);
            auto *retrieveFuture = storage.getFeeds();
            FeedCore::Feed *feed { nullptr };
            QObject::connect(retrieveFuture, &FeedCore::BaseFuture::finished, this, [&feed, retrieveFuture]{
                feed = retrieveFuture->result()[0];
            });
            QSignalSpy waitForRetrievedFeed(retrieveFuture, &FeedCore::BaseFuture::finished);
            waitForRetrievedFeed.wait();
            feed->setName("new name");
            QCoreApplication::processEvents();
        }
        {
            Sqlite::StorageImpl storage(testDbName);
            auto *retrieveFuture = storage.getFeeds();
            FeedCore::Feed *feed { nullptr };
            QObject::connect(retrieveFuture, &FeedCore::BaseFuture::finished, this, [&feed, retrieveFuture]{
                feed = retrieveFuture->result()[0];
            });
            QSignalSpy waitForRetrievedFeed(retrieveFuture, &FeedCore::BaseFuture::finished);
            waitForRetrievedFeed.wait();
            QVERIFY(feed->name() == "new name");
        }
    }
};

QTEST_MAIN(testStoreAndRetrieveFeed)

#include "tst_teststoreandretrievefeed.moc"
