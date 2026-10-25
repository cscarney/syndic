/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "articleref.h"
#include <QFuture>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <functional>

namespace FeedCore
{
class Context;
class Storage;
class Subscription;
class Readability;

/**
 * Push web content into the database
 */
class PageSaver : public QObject
{
    Q_OBJECT
public:
    /**
     * A feed found for a site
     */
    struct Discovery {
        QUrl url; /** < empty if nothing was found */
        QString name;
        bool isWebPage{false};
    };

    /**
     * Where a page would be saved
     */
    struct Resolution {
        ArticleRef article;
        QPointer<Subscription> suggestedFeed;
        Discovery discovered;
    };

    struct Request {
        QUrl url;
        QString html; /** < may be null */
        QString title;
        QString author;

        enum Target { NoFeed, ExistingFeed, SubscribeToFeed };
        Target target{NoFeed};

        QPointer<Subscription> feed; /** < for ExistingFeed */
        Discovery subscribe; /** < for SubscribeToFeed */
    };

    enum Error {
        NoError,
        InvalidUrl, /** < the request URL is not an absolute URL */
        ExtractionFailed, /** < readability couldn't extract any content from the page */
        FeedNotFound, /** < the target subscription no longer exists */
        SubscribeFailed, /** < the new subscription couldn't be stored */
        StorageFailed, /** < the article couldn't be stored */
    };

    struct Result {
        /** The stored article */
        ArticleRef article;

        /** True if we updated an existing article, false if we created a new one */
        bool updatedExisting{false};

        /** Why the page wasn't saved, when article is null */
        Error error{NoError};
    };

    explicit PageSaver(Context *context, Storage *storage, QObject *parent = nullptr);

    /**
     * Determine where the page would be saved. This may involve
     * network requests for feed discovery.
     */
    QFuture<Resolution> resolve(const QUrl &url);

    /**
     * Extract the readable content of a page and save it.
     */
    QFuture<Result> save(const Request &request);

    /**
     * Suggest a feed for a given article URL.
     */
    Subscription *bestFeedFor(const QUrl &url) const;

    /**
     * Find a subscription by its feed URL, or null.
     */
    Subscription *feedByUrl(const QUrl &url) const;

    /**
     * Try to find a feed for the site at /siteUrl/, including a scraped web page
     * feed. The resulting url is empty if nothing usable was found.
     */
    QFuture<Discovery> discover(const QUrl &siteUrl);

    /**
     * The URL that resolve() uses for feed discovery: the root of the page's site.
     */
    static QUrl siteRoot(const QUrl &pageUrl);

    /**
     * Override the readability implementation (by default, the context's).
     */
    void setReadability(Readability *readability);

    /**
     * Maximum time to wait for network operations (feed discovery, the first update of a new
     * subscription), in milliseconds.
     */
    void setTimeout(int msecs);

private:
    Context *m_context;
    Storage *m_storage;
    Readability *m_readability{nullptr};
    int m_timeout;

    Readability *readability();
    void extractContent(const Request &request, std::function<void(const QString &content)> callback);
    void storePage(const Request &request, const QString &content, std::function<void(const Result &)> callback);
    void storeInSubscription(const Request &request, const QString &content, std::function<void(const Result &)> callback);
};
}

Q_DECLARE_METATYPE(FeedCore::PageSaver::Resolution)
Q_DECLARE_METATYPE(FeedCore::PageSaver::Result)
Q_DECLARE_METATYPE(FeedCore::PageSaver::Discovery)
