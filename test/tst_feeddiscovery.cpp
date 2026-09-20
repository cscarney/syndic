/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feeddiscovery.h"
#include <QtTest>

using namespace FeedCore;

class testFeedDiscovery : public QObject
{
    Q_OBJECT

    static inline const QUrl kPageUrl{"https://example.org/blog/index.html"};

private slots:
    /**
     * An explicit <link rel="alternate"> outranks a feed-shaped anchor.
     */
    void testExplicitFeedLinkBeatsLaterAnchor()
    {
        const QByteArray html("<html><head>"
                              "<link rel=\"alternate\" type=\"application/rss+xml\" href=\"/correct.xml\">"
                              "</head><body>"
                              "<a href=\"/wrong.xml\">Subscribe</a>"
                              "</body></html>");

        QCOMPARE(FeedDiscovery::discoverFeed(kPageUrl, html), QUrl("https://example.org/correct.xml"));
    }
};

QTEST_MAIN(testFeedDiscovery)

#include "tst_feeddiscovery.moc"
