/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "article.h"
#include "context.h"
#include "future.h"
#include "pageSaverTemplates.h"
#include "pagesaver.h"
#include "provisionalsubscription.h"
#include "readability/readability.h"
#include "readability/readabilityresult.h"
#include "sqlite/storageimpl.h"
#include "subscription.h"
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

using namespace FeedCore;

namespace
{
class MockResult : public ReadabilityResult
{
public:
    explicit MockResult(QObject *parent)
        : ReadabilityResult(parent)
    {
    }

    void emitLater(const QString &content)
    {
        QMetaObject::invokeMethod(
            this,
            [this, content] {
                if (content.isEmpty()) {
                    emit error();
                } else {
                    emit finished(content);
                }
                deleteLater();
            },
            Qt::QueuedConnection);
    }
};

class MockReadability : public Readability
{
public:
    QString lastHtml;
    QUrl lastFetchUrl;

    ReadabilityResult *fetch(const QUrl &url) override
    {
        lastFetchUrl = url;
        auto *result = new MockResult(this);
        result->emitLater(QStringLiteral("<p>fetched</p>"));
        return result;
    }

    ReadabilityResult *parse(const QString &html, const QUrl & /*url*/) override
    {
        lastHtml = html;
        auto *result = new MockResult(this);
        result->emitLater(html.trimmed().isEmpty() ? QString() : QStringLiteral("<p>readable:%1</p>").arg(html));
        return result;
    }
};

}

class testPageSaver : public QObject
{
    Q_OBJECT

    std::unique_ptr<QTemporaryDir> m_dir;
    Context *m_context{nullptr};
    MockReadability *m_readability{nullptr};
    PageSaver *m_saver{nullptr};

    QString path(const QString &name)
    {
        return m_dir->filePath(name);
    }

    void openContext()
    {
        delete m_context;
        auto *storage = new SqliteStorage::StorageImpl(path("test.db"));
        m_context = new Context(storage);
        QVERIFY(QTest::qWaitFor([this] {
            return m_context->feedListComplete();
        }));
        m_readability = new MockReadability;
        m_readability->setParent(m_context);
        m_saver = new PageSaver(m_context, storage, m_context);
        m_saver->setReadability(m_readability);
        m_saver->setTimeout(5000);
    }

    QUrl writeFeed(const QList<int> &posts)
    {
        QString entries;
        for (int post : posts) {
            entries += QString(kEntryTemplate).arg(post);
        }
        QFile file(path("feed.atom"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qFatal("couldn't write feed");
        }
        file.write(QString(kFeedTemplate).arg(entries).toUtf8());
        return QUrl::fromLocalFile(file.fileName());
    }

    template<typename T>
    static T await(QFuture<T> future)
    {
        if (!QTest::qWaitFor(
                [&future] {
                    return future.isFinished();
                },
                10000)) {
            qWarning() << "future timed out";
            return T();
        }
        return future.resultCount() > 0 ? future.result() : T();
    }

    template<typename T>
    static QList<T> awaitAll(QFuture<T> future)
    {
        if (!QTest::qWaitFor([&future] {
                return future.isFinished();
            })) {
            qWarning() << "future timed out";
        }
        return Future::safeResults(future);
    }

    static void waitForUpdate(Subscription *feed)
    {
        if (!QTest::qWaitFor([feed] {
                return feed->status() != Feed::Updating;
            })) {
            qWarning() << "update timed out";
        }
        // let the storage thread finish writing
        QTest::qWait(50);
    }

    Subscription *addFeed(const QUrl &url)
    {
        ProvisionalSubscription provisional;
        provisional.setUrl(url);
        QSignalSpy spy(&provisional, &ProvisionalSubscription::targetFeedChanged);
        m_context->addFeed(&provisional);
        if (!spy.wait()) {
            return nullptr;
        }
        Subscription *feed = provisional.targetFeed();
        feed->update();
        waitForUpdate(feed);
        return feed;
    }

    QString readableContent(const ArticleRef &article)
    {
        return await(article->getCachedReadableContent());
    }

    PageSaver::Result save(const QString &url, PageSaver::Request::Target target, Subscription *feed = nullptr, const QString &html = "<p>dom</p>")
    {
        PageSaver::Request request;
        request.url = QUrl(url);
        request.html = html;
        request.title = QStringLiteral("Saved title");
        request.target = target;
        request.feed = feed;
        return await(m_saver->save(request));
    }

private slots:
    void init()
    {
        m_dir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_dir->isValid());
        openContext();
    }

    void cleanup()
    {
        delete m_context;
        m_context = nullptr;
        m_dir.reset();
    }

    void testExistingArticleGetsReadableContent()
    {
        Subscription *feed = addFeed(writeFeed({1, 2}));
        QVERIFY(feed);

        auto result = save("https://www.example.com/posts/1?utm_source=rss", PageSaver::Request::NoFeed);
        QVERIFY(result.article);
        QVERIFY(result.updatedExisting);
        QCOMPARE(result.article->feed(), feed);
        QCOMPARE(result.article->title(), QStringLiteral("Post 1"));
        QVERIFY(result.article->isStarred());
        QVERIFY(result.article->preferReadable());
        QCOMPARE(readableContent(result.article), QStringLiteral("<p>readable:<p>dom</p></p>"));
        QCOMPARE(awaitAll(feed->getArticles(false)).size(), 2);
    }

    void testResolveExistingArticle()
    {
        Subscription *feed = addFeed(writeFeed({1}));
        auto resolution = await(m_saver->resolve(QUrl("https://example.com/posts/1/")));
        QVERIFY(resolution.article);
        QCOMPARE(resolution.suggestedFeed.data(), feed);
    }

    void testResolveSuggestsFeedBySiteLink()
    {
        Subscription *feed = addFeed(writeFeed({1}));
        QCOMPARE(feed->link(), QUrl("http://example.com/"));
        auto resolution = await(m_saver->resolve(QUrl("https://example.com/posts/99")));
        QVERIFY(!resolution.article);
        QCOMPARE(resolution.suggestedFeed.data(), feed);
        QVERIFY(resolution.discovered.url.isEmpty());
    }

    void testSaveIntoFeedThenMerge()
    {
        Subscription *feed = addFeed(writeFeed({1, 2}));

        auto result = save("https://example.com/posts/3", PageSaver::Request::ExistingFeed, feed);
        QVERIFY(result.article);
        QVERIFY(!result.updatedExisting);
        QCOMPARE(result.article->feed(), feed);
        QVERIFY(result.article->isStarred());
        QCOMPARE(result.article->title(), QStringLiteral("Saved title"));
        QCOMPARE(readableContent(result.article), QStringLiteral("<p>readable:<p>dom</p></p>"));
        QCOMPARE(awaitAll(feed->getArticles(false)).size(), 3);

        // the post shows up in the feed later; it should be merged into the saved article
        writeFeed({1, 2, 3});
        feed->update();
        waitForUpdate(feed);

        const auto matches = awaitAll(m_context->getStarred());
        QCOMPARE(matches.size(), 1);
        QCOMPARE(awaitAll(feed->getArticles(false)).size(), 3);
        const ArticleRef merged = matches.first();
        QCOMPARE(merged->title(), QStringLiteral("Post 3"));
        QVERIFY(merged->isStarred());
        QVERIFY(merged->preferReadable());
        QCOMPARE(readableContent(merged), QStringLiteral("<p>readable:<p>dom</p></p>"));

        // and it should stay merged on subsequent updates
        feed->update();
        waitForUpdate(feed);
        QCOMPARE(awaitAll(feed->getArticles(false)).size(), 3);
    }

    void testFeedlessArticle()
    {
        auto result = save("https://other.example.org/article", PageSaver::Request::NoFeed);
        QVERIFY(result.article);
        QVERIFY(qobject_cast<Subscription *>(result.article->feed()) == nullptr);
        QCOMPARE(result.article->author(), QStringLiteral("other.example.org"));
        QVERIFY(result.article->isRead());
        QCOMPARE(awaitAll(m_context->getStarred()).size(), 1);

        // survives a restart
        result = {};
        openContext();
        auto starred = awaitAll(m_context->getStarred());
        QCOMPARE(starred.size(), 1);
        QVERIFY(qobject_cast<Subscription *>(starred.first()->feed()) == nullptr);

        // un-starring keeps it, like any other article
        starred.first()->setStarred(false);
        QTest::qWait(50);
        starred.clear();
        openContext();
        QCOMPARE(awaitAll(m_context->getStarred()).size(), 0);
        QCOMPARE(awaitAll(m_context->getArticles(false)).size(), 1);
    }

    void testFetchWhenNoHtml()
    {
        auto result = save("https://other.example.org/article", PageSaver::Request::NoFeed, nullptr, QString());
        QVERIFY(result.article);
        QCOMPARE(m_readability->lastFetchUrl, QUrl("https://other.example.org/article"));
        QCOMPARE(readableContent(result.article), QStringLiteral("<p>fetched</p>"));
    }

    void testDefaultTitle()
    {
        PageSaver::Request request;
        request.url = QUrl("https://other.example.org/article");
        request.html = "<html><head><title>Ignored</title></head><body>x</body></html>";
        auto result = await(m_saver->save(request));
        QVERIFY(result.article);
        QCOMPARE(result.article->title(), QStringLiteral("https://other.example.org/article"));
    }

    void testReadabilityFailure()
    {
        auto result = save("https://other.example.org/article", PageSaver::Request::NoFeed, nullptr, " ");
        QVERIFY(!result.article);
        QCOMPARE(result.error, PageSaver::ExtractionFailed);
        QCOMPARE(awaitAll(m_context->getStarred()).size(), 0);
    }

    void testDiscover()
    {
        const QUrl feedUrl = writeFeed({1});
        QFile page(path("index.html"));
        QVERIFY(page.open(QIODevice::WriteOnly));
        page.write(kDiscoveryPage);
        page.close();

        auto discovery = await(m_saver->discover(QUrl::fromLocalFile(page.fileName())));
        QCOMPARE(discovery.url, feedUrl);
        QCOMPARE(discovery.name, QStringLiteral("Example Feed"));
        QVERIFY(!discovery.isWebPage);
    }

    void testSubscribeAndMerge()
    {
        const QUrl feedUrl = writeFeed({1, 2});
        PageSaver::Request request;
        request.url = QUrl("https://example.com/posts/2");
        request.html = "<p>dom</p>";
        request.target = PageSaver::Request::SubscribeToFeed;
        request.subscribe.url = feedUrl;
        request.subscribe.name = "Discovered";
        auto result = await(m_saver->save(request));
        QVERIFY(result.article);
        QVERIFY(result.updatedExisting);
        auto *feed = qobject_cast<Subscription *>(result.article->feed());
        QVERIFY(feed);
        QCOMPARE(feed->url(), feedUrl);
        QCOMPARE(feed->name(), QStringLiteral("Discovered"));
        QCOMPARE(m_context->getFeeds().size(), 1);
        QCOMPARE(awaitAll(feed->getArticles(false)).size(), 2);
        QCOMPARE(awaitAll(m_context->getStarred()).size(), 1);
    }

    void testSubscribeNewArticle()
    {
        const QUrl feedUrl = writeFeed({1});
        PageSaver::Request request;
        request.url = QUrl("https://example.com/posts/5");
        request.html = "<p>dom</p>";
        request.target = PageSaver::Request::SubscribeToFeed;
        request.subscribe.url = feedUrl;
        auto result = await(m_saver->save(request));
        QVERIFY(result.article);
        QVERIFY(!result.updatedExisting);
        auto *feed = qobject_cast<Subscription *>(result.article->feed());
        QVERIFY(feed);
        QCOMPARE(awaitAll(feed->getArticles(false)).size(), 2);
    }

    void testMigrationBackfillsUrlKeys()
    {
        delete m_context;
        m_context = nullptr;
        m_saver = nullptr;
        const QString dbPath = path("old.db");
        {
            auto db = QSqlDatabase::addDatabase("QSQLITE", "migration");
            db.setDatabaseName(dbPath);
            QVERIFY(db.open());
            QSqlQuery q(db);
            QVERIFY(
                q.exec("CREATE TABLE Feed(id INTEGER PRIMARY KEY, displayName TEXT, category TEXT, url TEXT NOT NULL, link TEXT, icon TEXT, "
                       "updateInterval INTEGER, lastUpdate INTEGER, expireAge INTEGER, flags INTEGER)"));
            QVERIFY(
                q.exec("CREATE TABLE Item(id INTEGER PRIMARY KEY, feed INTEGER REFERENCES Feed, localId TEXT NOT NULL, headline TEXT, author TEXT, "
                       "date INTEGER, url TEXT, feedContent TEXT, isRead INTEGER, isStarred INTEGER, readableContent TEXT, UNIQUE(feed,localId))"));
            QVERIFY(q.exec("INSERT INTO Feed(id, displayName, url) VALUES (1, 'Old', 'http://example.com/feed')"));
            QVERIFY(
                q.exec("INSERT INTO Item(feed, localId, headline, date, url, isRead, isStarred) "
                       "VALUES (1, 'x', 'Old post', 0, 'http://example.com/old-post', 0, 0)"));
            QVERIFY(q.exec("PRAGMA user_version = 2"));
            db.close();
        }
        QSqlDatabase::removeDatabase("migration");

        auto *storage = new SqliteStorage::StorageImpl(dbPath);
        m_context = new Context(storage);
        QVERIFY(QTest::qWaitFor([this] {
            return m_context->feedListComplete();
        }));
        PageSaver saver(m_context, storage);
        auto resolution = await(saver.resolve(QUrl("https://www.example.com/old-post/")));
        QVERIFY(resolution.article);
        QCOMPARE(resolution.article->title(), QStringLiteral("Old post"));
    }
};

QTEST_GUILESS_MAIN(testPageSaver)
#include "tst_pagesaver.moc"
