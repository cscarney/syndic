/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "networkaccessmanager.h"
#include "provisionalfeed.h"
#include "updatablefeed.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSignalSpy>
#include <QtTest>
#include <Syndication/Feed>
#include <Syndication/ParserCollection>

using namespace FeedCore;

// Mock network reply class
class MockNetworkReply : public QNetworkReply
{
public:
    explicit MockNetworkReply(QObject *parent = nullptr)
        : QNetworkReply(parent)
    {
    }

    void setData(const QByteArray &data)
    {
        m_data = data;
        m_offset = 0;
    }

    void setUrl(const QUrl &url)
    {
        QNetworkReply::setUrl(url);
    }

    void setError(QNetworkReply::NetworkError error, const QString &errorString)
    {
        QNetworkReply::setError(error, errorString);
    }

    void finish()
    {
        emit finished();
    }

    bool open(OpenMode mode) override
    {
        if (mode != ReadOnly) {
            return false;
        }

        setOpenMode(ReadOnly);
        return QNetworkReply::open(mode);
    }

    void abort() override
    {
        setError(OperationCanceledError, "Aborted");
        emit finished();
    }
    qint64 bytesAvailable() const override
    {
        return m_data.size() - m_offset;
    }
    bool isSequential() const override
    {
        return true;
    }

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        if (m_offset >= m_data.size())
            return -1;

        qint64 bytesToRead = qMin(maxSize, static_cast<qint64>(m_data.size() - m_offset));
        memcpy(data, m_data.constData() + m_offset, bytesToRead);
        m_offset += bytesToRead;
        return bytesToRead;
    }

private:
    QByteArray m_data;
    qint64 m_offset = 0;
};

// Mock network access manager for testing
class MockNetworkAccessManager : public FeedCore::NetworkAccessManager
{
public:
    explicit MockNetworkAccessManager(QObject *parent = nullptr)
        : FeedCore::NetworkAccessManager(parent)
    {
    }

    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData = nullptr) override
    {
        Q_UNUSED(op);
        Q_UNUSED(outgoingData);

        if (!m_responses.contains(request.url())) {
            auto *reply = new MockNetworkReply(this);
            reply->setUrl(request.url());
            reply->setError(QNetworkReply::ContentNotFoundError, "URL not found in mock responses");
            QTimer::singleShot(0, reply, &MockNetworkReply::finish);
            return reply;
        }

        auto *reply = new MockNetworkReply(this);
        reply->setUrl(request.url());
        reply->setData(m_responses[request.url()]);
        reply->open(QIODevice::ReadOnly);
        QTimer::singleShot(0, reply, &MockNetworkReply::finish);
        return reply;
    }

    void addResponse(const QUrl &url, const QByteArray &data)
    {
        m_responses[url] = data;
    }

private:
    QMap<QUrl, QByteArray> m_responses;
};

class TestWebPageFallback : public QObject
{
    Q_OBJECT

private:
    QPointer<MockNetworkAccessManager> m_networkManager;
    QScopedPointer<UpdatableFeed> m_feed;

    // Sample data
    const QByteArray m_atomFeedData = QByteArray(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<feed xmlns=\"http://www.w3.org/2005/Atom\">"
        "  <title>Test Feed</title>"
        "  <link href=\"https://example.org/\"/>"
        "  <updated>2023-01-01T12:00:00Z</updated>"
        "  <id>urn:uuid:60a76c80-d399-11d9-b93C-0003939e0af6</id>"
        "  <entry>"
        "    <title>Sample Article 1</title>"
        "    <link href=\"https://example.org/article1\"/>"
        "    <id>article1-id</id>"
        "    <updated>2023-01-01T12:00:00Z</updated>"
        "    <summary>Sample content 1</summary>"
        "  </entry>"
        "</feed>");

    const QByteArray m_webPageWithFeedLinksData = QByteArray(
        "<html><head>"
        "<link rel=\"alternate\" type=\"application/rss+xml\" href=\"https://example.org/feed.xml\">"
        "</head><body>"
        "<h1>Example Website</h1>"
        "<p>This is a sample web page with a feed link.</p>"
        "</body></html>");

    const QByteArray m_webPageWithArticleLinksData = QByteArray(
        "<html><head><title>Example News Site</title></head><body>"
        "<h1>Latest Articles</h1>"
        "<ul>"
        "  <li><a href=\"https://example.org/article1\">Article 1 Title</a></li>"
        "  <li><a href=\"https://example.org/article2\">Article 2 Title</a></li>"
        "  <li><a href=\"https://example.org/about\">About Us</a></li>"
        "</ul>"
        "</body></html>");

    void initTestCase()
    {
        qRegisterMetaType<FeedCore::Feed::LoadStatus>();
        qRegisterMetaType<FeedCore::Feed *>();
    }

    void init()
    {
        m_networkManager = new MockNetworkAccessManager();
        FeedCore::NetworkAccessManager::setInstance(m_networkManager.get());
        m_feed.reset(new ProvisionalFeed());
    }

    void cleanup()
    {
        m_feed.reset();
    }

    // Test Case A: Feed URLs that point to actual RSS/Atom feeds should load feed content
    void testDirectFeed()
    {
        // Set up test data
        QUrl feedUrl("https://example.org/feed.xml");
        m_networkManager->addResponse(feedUrl, m_atomFeedData);

        // Set up signal spies
        QSignalSpy statusSpy(m_feed.get(), &Feed::statusChanged);

        // Set the feed URL and start the update
        m_feed->setUrl(feedUrl);
        m_feed->updater()->start();

        // Wait for the update to complete
        QTest::qWait(500);

        // Verify the feed loaded correctly
        QCOMPARE(m_feed->status(), Feed::Idle);
        QCOMPARE(m_feed->name(), QString("Test Feed"));
        QCOMPARE(m_feed->flags() & Feed::IsWebPageFlag, 0); // Should not have web page flag

        // Verify status changes: Idle -> Updating -> Idle
        QCOMPARE(statusSpy.count(), 2);
    }

    // Test Case B: Feed URLs pointing to web pages should try to discover feeds first
    void testWebPageWithFeedDiscovery()
    {
        // Set up test data for a web page with feed discovery link
        QUrl webpageUrl("https://example.org/blog");
        QUrl discoveredFeedUrl("https://example.org/feed.xml");

        m_networkManager->addResponse(webpageUrl, m_webPageWithFeedLinksData);
        m_networkManager->addResponse(discoveredFeedUrl, m_atomFeedData);
        m_feed->setUrl(webpageUrl);

        // Set up signal spies
        QSignalSpy statusSpy(m_feed.get(), &Feed::statusChanged);
        QSignalSpy urlChangedSpy(m_feed.get(), &Feed::urlChanged);

        // Set the feed URL and start the update
        m_feed->updater()->start();

        // Wait for the update to complete
        QTest::qWait(500);

        // Verify the feed loaded correctly and URL was changed to the discovered feed
        QCOMPARE(m_feed->status(), Feed::Idle);
        QCOMPARE(m_feed->name(), QString("Test Feed"));
        QCOMPARE(m_feed->url(), discoveredFeedUrl);
        QCOMPARE(m_feed->flags() & Feed::IsWebPageFlag, 0); // Should not have web page flag

        // Verify URL changed (from webpage to feed URL)
        QCOMPARE(urlChangedSpy.count(), 1);
    }

    // Test Case C: Web pages with no discoverable feeds or failed feed loads should fall back to web page parsing
    void testWebPageFallback()
    {
        // Set up test data for a web page with no feed links
        QUrl webpageUrl("https://example.org/news");

        m_networkManager->addResponse(webpageUrl, m_webPageWithArticleLinksData);

        // Set up signal spies
        QSignalSpy statusSpy(m_feed.get(), &Feed::statusChanged);
        QSignalSpy flagsChangedSpy(m_feed.get(), &Feed::flagsChanged);

        // Set the feed URL and start the update
        m_feed->setUrl(webpageUrl);
        m_feed->updater()->start();

        // Wait for the update to complete
        QTest::qWait(500);

        // Verify the feed was processed as a web page
        QCOMPARE(m_feed->status(), Feed::Idle);
        QCOMPARE(m_feed->flags() & Feed::IsWebPageFlag, Feed::IsWebPageFlag); // Should have web page flag

        // Verify flags changed (indicating web page mode was set)
        QCOMPARE(flagsChangedSpy.count(), 1);
    }

    // Test Case C-2: Web page with discoverable feed, but feed fails to load
    void testWebPageFallbackWhenFeedLoadFails()
    {
        // Set up test data for a web page with feed discovery link
        QUrl webpageUrl("https://example.org/news");
        QUrl discoveredFeedUrl("https://example.org/feed.xml");

        // Add the web page response
        m_networkManager->addResponse(webpageUrl, m_webPageWithFeedLinksData);

        // But make the discovered feed URL fail (not adding a response for it)
        // The MockNetworkAccessManager will return a ContentNotFoundError for this URL

        // Set up signal spies
        QSignalSpy statusSpy(m_feed.get(), &Feed::statusChanged);
        QSignalSpy flagsChangedSpy(m_feed.get(), &Feed::flagsChanged);

        // Set the feed URL and start the update
        m_feed->setUrl(webpageUrl);
        m_feed->updater()->start();

        // Wait for the update to complete
        QTest::qWait(500);

        // Verify the feed was processed as a web page after feed loading failed
        QCOMPARE(m_feed->status(), Feed::Idle);
        QCOMPARE(m_feed->flags() & Feed::IsWebPageFlag, Feed::IsWebPageFlag); // Should have web page flag

        // Verify flags changed (indicating web page mode was set)
        QCOMPARE(flagsChangedSpy.count(), 1);
    }
};

QTEST_MAIN(TestWebPageFallback)
#include "tst_webpage_fallback.moc"
