/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "articlelinkextractor.h"
#include <QtTest>
#include <cstring>
#include <new>

using namespace FeedCore;

class testArticleLinkExtractor : public QObject
{
    Q_OBJECT

    static constexpr const char *kTwoDatedArticles = "<html><head><title>Example News</title></head><body>"
                                                     "<a href=\"/2026/01/15/first-story\">First story headline</a>"
                                                     "<a href=\"/2026/02/20/second-story\">Second story headline</a>"
                                                     "</body></html>";

private slots:
    void testDatedLinkExtraction()
    {
        const QUrl baseUrl("https://example.org/");
        const QByteArray html(kTwoDatedArticles);

        ArticleLinkExtractor extractor(QString::fromUtf8(html), baseUrl);
        extractor.walk();
        const QList<ArticleLink> links = extractor.articleLinks();
        QCOMPARE(extractor.articleLinks().size(), 2);
        QCOMPARE(links.at(0).url, QUrl("https://example.org/2026/01/15/first-story"));
        QCOMPARE(links.at(1).url, QUrl("https://example.org/2026/02/20/second-story"));
    }

    /* Regression Test: Starting score threshold was uninitialized */
    void testUninitializedScoreThresholdRegression()
    {
        const QUrl baseUrl("https://example.org/");
        const QByteArray html(kTwoDatedArticles);

        // Allocate over deterministic garbage so we fail with uninitialized members
        alignas(ArticleLinkExtractor) unsigned char storage[sizeof(ArticleLinkExtractor)];
        std::memset(storage, 0x7f, sizeof(storage));
        auto *fromBytes = new (storage) ArticleLinkExtractor(html, baseUrl);
        fromBytes->walk();
        const QList<ArticleLink> links = fromBytes->articleLinks();
        fromBytes->~ArticleLinkExtractor();

        QCOMPARE(links.size(), 2);
        QCOMPARE(links.at(0).url, QUrl("https://example.org/2026/01/15/first-story"));
        QCOMPARE(links.at(1).url, QUrl("https://example.org/2026/02/20/second-story"));
    }
};

QTEST_MAIN(testArticleLinkExtractor)

#include "tst_articlelinkextractor.moc"
