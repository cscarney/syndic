/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "urlmatch.h"
#include <QtTest>

using namespace FeedCore;

class testUrlMatch : public QObject
{
    Q_OBJECT

private slots:
    void testEquivalentUrls_data()
    {
        QTest::addColumn<QString>("a");
        QTest::addColumn<QString>("b");
        QTest::newRow("scheme") << "http://example.com/post" << "https://example.com/post";
        QTest::newRow("www") << "https://www.example.com/post" << "https://example.com/post";
        QTest::newRow("mobile") << "https://m.example.com/post" << "https://example.com/post";
        QTest::newRow("host case") << "https://EXAMPLE.com/post" << "https://example.com/post";
        QTest::newRow("trailing slash") << "https://example.com/post/" << "https://example.com/post";
        QTest::newRow("fragment") << "https://example.com/post#comments" << "https://example.com/post";
        QTest::newRow("default port") << "https://example.com:443/post" << "https://example.com/post";
        QTest::newRow("index page") << "https://example.com/post/index.html" << "https://example.com/post";
        QTest::newRow("amp suffix") << "https://example.com/post/amp" << "https://example.com/post";
        QTest::newRow("utm") << "https://example.com/post?utm_source=rss&utm_medium=feed" << "https://example.com/post";
        QTest::newRow("fbclid") << "https://example.com/post?fbclid=abc" << "https://example.com/post";
        QTest::newRow("param order") << "https://example.com/post?b=2&a=1" << "https://example.com/post?a=1&b=2";
        QTest::newRow("tracking mixed") << "https://example.com/?p=12&utm_campaign=x" << "https://example.com/?p=12";
    }

    void testEquivalentUrls()
    {
        QFETCH(QString, a);
        QFETCH(QString, b);
        QCOMPARE(UrlMatch::normalizedUrlKey(QUrl(a)), UrlMatch::normalizedUrlKey(QUrl(b)));
    }

    void testDistinctUrls_data()
    {
        QTest::addColumn<QString>("a");
        QTest::addColumn<QString>("b");
        QTest::newRow("path") << "https://example.com/post-1" << "https://example.com/post-2";
        QTest::newRow("host") << "https://example.com/post" << "https://example.org/post";
        QTest::newRow("subdomain") << "https://blog.example.com/post" << "https://example.com/post";
        QTest::newRow("query") << "https://example.com/?p=1" << "https://example.com/?p=2";
        QTest::newRow("port") << "https://example.com:8080/post" << "https://example.com/post";
    }

    void testDistinctUrls()
    {
        QFETCH(QString, a);
        QFETCH(QString, b);
        QVERIFY(UrlMatch::normalizedUrlKey(QUrl(a)) != UrlMatch::normalizedUrlKey(QUrl(b)));
    }

    void testNoHost()
    {
        QVERIFY(UrlMatch::normalizedUrlKey(QUrl()).isEmpty());
        QVERIFY(UrlMatch::normalizedUrlKey(QUrl("about:blank")).isEmpty());
    }

    void testSiteMatchScore_data()
    {
        QTest::addColumn<QString>("page");
        QTest::addColumn<QString>("site");
        QTest::addColumn<int>("score");
        QTest::newRow("root") << "https://example.com/2024/01/post" << "https://example.com/" << 0;
        QTest::newRow("root www") << "https://example.com/post" << "http://www.example.com" << 0;
        QTest::newRow("section") << "https://example.com/blog/post" << "https://example.com/blog/" << 1;
        QTest::newRow("other section") << "https://example.com/news/post" << "https://example.com/blog/" << -1;
        QTest::newRow("partial segment") << "https://example.com/blogger/post" << "https://example.com/blog" << -1;
        QTest::newRow("other host") << "https://example.org/post" << "https://example.com/" << -1;
        QTest::newRow("empty site") << "https://example.org/post" << "" << -1;
    }

    void testSiteMatchScore()
    {
        QFETCH(QString, page);
        QFETCH(QString, site);
        QFETCH(int, score);
        QCOMPARE(UrlMatch::siteMatchScore(QUrl(page), QUrl(site)), score);
    }
};

QTEST_GUILESS_MAIN(testUrlMatch)
#include "tst_urlmatch.moc"
