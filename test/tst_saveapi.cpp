/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "context.h"
#include "pagesaver.h"
#include "readability/readability.h"
#include "readability/readabilityresult.h"
#include "saveapi.h"
#include "sqlite/storageimpl.h"
#include <QJsonArray>
#include <QTemporaryDir>
#include <QtTest>

using namespace FeedCore;

namespace
{
class EchoResult : public ReadabilityResult
{
public:
    EchoResult(QObject *parent, const QString &content)
        : ReadabilityResult(parent)
    {
        QMetaObject::invokeMethod(
            this,
            [this, content] {
                emit finished(content);
                deleteLater();
            },
            Qt::QueuedConnection);
    }
};

class EchoReadability : public Readability
{
public:
    ReadabilityResult *fetch(const QUrl &url) override
    {
        return new EchoResult(this, url.toString());
    }
    ReadabilityResult *parse(const QString &html, const QUrl & /*url*/) override
    {
        return new EchoResult(this, html);
    }
};
}

class testSaveApi : public QObject
{
    Q_OBJECT

    std::unique_ptr<QTemporaryDir> m_dir;
    Context *m_context{nullptr};
    SaveApi *m_api{nullptr};

    static SaveApi::Response await(QFuture<SaveApi::Response> future)
    {
        if (!QTest::qWaitFor(
                [&future] {
                    return future.isFinished();
                },
                10000)) {
            return {0, {}};
        }
        return future.result();
    }

    SaveApi::Response resolve(const QJsonObject &body)
    {
        return await(m_api->resolve(body));
    }

    SaveApi::Response save(const QJsonObject &body)
    {
        return await(m_api->save(body));
    }

private slots:
    void init()
    {
        m_dir = std::make_unique<QTemporaryDir>();
        auto *storage = new SqliteStorage::StorageImpl(m_dir->filePath("test.db"));
        m_context = new Context(storage);
        QVERIFY(QTest::qWaitFor([this] {
            return m_context->feedListComplete();
        }));
        auto *readability = new EchoReadability;
        readability->setParent(m_context);
        auto *saver = new PageSaver(m_context, storage, m_context);
        saver->setReadability(readability);
        m_api = new SaveApi(m_context, saver, m_context);
    }

    void cleanup()
    {
        delete m_context;
        m_context = nullptr;
        m_dir.reset();
    }

    void testPing()
    {
        auto response = SaveApi::ping(true);
        QCOMPARE(response.status, 200);
        QCOMPARE(response.body["app"].toString(), QStringLiteral("syndic"));
        QCOMPARE(response.body["apiVersion"].toInt(), SaveApi::kApiVersion);
        QCOMPARE(response.body["paired"].toBool(), true);
        QCOMPARE(SaveApi::ping(false).body["paired"].toBool(), false);
    }

    void testInvalidUrl()
    {
        QCOMPARE(resolve({{"url", "not a url"}}).status, 400);
        QCOMPARE(resolve({}).status, 400);
        QCOMPARE(save({{"url", "/relative"}}).status, 400);
    }

    void testInvalidFeed()
    {
        auto response = save({{"url", "https://example.com/a"}, {"html", "<p>x</p>"}, {"feed", "https://example.com/nope.xml"}});
        QCOMPARE(response.status, 400);
        QCOMPARE(response.body["error"].toString(), QStringLiteral("invalid-feed"));
        QCOMPARE(save({{"url", "https://example.com/a"}, {"feed", 12}}).status, 400);
    }

    void testExtractionFailed()
    {
        auto response = save({{"url", "https://example.com/a"}, {"html", "   "}});
        QCOMPARE(response.status, 422);
        QCOMPARE(response.body["error"].toString(), QStringLiteral("extraction-failed"));
        QVERIFY(!response.body.contains("message"));
    }

    void testSaveAndResolveFeedless()
    {
        auto response = save({{"url", "https://example.com/a"}, {"html", "<p>x</p>"}, {"title", "  A title "}, {"feed", QJsonValue::Null}});
        QCOMPARE(response.status, 200);
        QCOMPARE(response.body["title"].toString(), QStringLiteral("A title"));
        QVERIFY(response.body["feed"].isNull());
        QCOMPARE(response.body["updatedExisting"].toBool(), false);

        response = resolve({{"url", "http://www.example.com/a/"}});
        QCOMPARE(response.status, 200);
        QCOMPARE(response.body["article"].toObject()["title"].toString(), QStringLiteral("A title"));
        QVERIFY(response.body["feeds"].isArray());
        QVERIFY(!response.body.contains("discovered"));

        response = save({{"url", "https://example.com/a"}, {"html", "<p>y</p>"}});
        QCOMPARE(response.status, 200);
        QCOMPARE(response.body["updatedExisting"].toBool(), true);
    }

    void testAllowedOrigin_data()
    {
        QTest::addColumn<QByteArray>("origin");
        QTest::addColumn<bool>("allowed");
        QTest::newRow("none") << QByteArray() << true;
        QTest::newRow("firefox") << QByteArray("moz-extension://1234-5678") << true;
        QTest::newRow("chrome") << QByteArray("chrome-extension://abcdef") << true;
        QTest::newRow("web") << QByteArray("https://evil.example") << false;
        QTest::newRow("http") << QByteArray("http://127.0.0.1:8080") << false;
        QTest::newRow("null") << QByteArray("null") << false;
    }

    void testAllowedOrigin()
    {
        QFETCH(QByteArray, origin);
        QFETCH(bool, allowed);
        QCOMPARE(SaveApi::isAllowedOrigin(origin), allowed);
    }

    void testAllowedHost_data()
    {
        QTest::addColumn<QByteArray>("host");
        QTest::addColumn<bool>("allowed");
        QTest::newRow("ipv4") << QByteArray("127.0.0.1:4242") << true;
        QTest::newRow("localhost") << QByteArray("LocalHost:4242") << true;
        QTest::newRow("ipv6") << QByteArray("[::1]:4242") << true;
        QTest::newRow("wrong port") << QByteArray("127.0.0.1:4243") << false;
        QTest::newRow("no port") << QByteArray("localhost") << false;
        QTest::newRow("rebinding") << QByteArray("evil.example:4242") << false;
        QTest::newRow("empty") << QByteArray() << false;
    }

    void testAllowedHost()
    {
        QFETCH(QByteArray, host);
        QFETCH(bool, allowed);
        QCOMPARE(SaveApi::isAllowedHost(host, 4242), allowed);
    }
};

QTEST_GUILESS_MAIN(testSaveApi)
#include "tst_saveapi.moc"
