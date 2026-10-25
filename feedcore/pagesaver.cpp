/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "pagesaver.h"
#include "article.h"
#include "context.h"
#include "future.h"
#include "provisionalsubscription.h"
#include "readability/readability.h"
#include "readability/readabilityresult.h"
#include "storage.h"
#include "subscription.h"
#include "urlmatch.h"
#include <QTimer>
#include <functional>
#include <memory>
#include <utility>

using namespace FeedCore;

static constexpr const int kDefaultTimeoutMsecs = 30000;

namespace
{
/**
 * Wrap /func/ so that only the first call (by any copy of the wrapper) is passed through.
 */
template<typename Func>
auto once(Func func)
{
    return [func, done = std::make_shared<bool>(false)] {
        if (!std::exchange(*done, true)) {
            func();
        }
    };
}

/**
 * Call /callback/ once when /feed/ is not updating, or after /timeout/ msecs.
 */
void whenNotUpdating(Subscription *feed, int timeout, QObject *context, std::function<void()> callback)
{
    if (feed->status() != Feed::Updating) {
        QMetaObject::invokeMethod(context, callback, Qt::QueuedConnection);
        return;
    }
    auto *guard = new QObject(context);
    auto fire = once([guard, callback] {
        guard->deleteLater();
        callback();
    });
    QObject::connect(feed, &Feed::statusChanged, guard, [feed, fire] {
        if (feed->status() != Feed::Updating) {
            fire();
        }
    });
    QObject::connect(feed, &QObject::destroyed, guard, [fire] {
        fire();
    });
    QTimer::singleShot(timeout, guard, [fire] {
        fire();
    });
}
}

PageSaver::PageSaver(Context *context, Storage *storage, QObject *parent)
    : QObject(parent)
    , m_context(context)
    , m_storage(storage)
    , m_timeout(kDefaultTimeoutMsecs)
{
}

void PageSaver::setReadability(Readability *readability)
{
    m_readability = readability;
}

void PageSaver::setTimeout(int msecs)
{
    m_timeout = msecs;
}

Readability *PageSaver::readability()
{
    return m_readability ? m_readability : m_context->getReadability();
}

QUrl PageSaver::siteRoot(const QUrl &pageUrl)
{
    QUrl root;
    root.setScheme(pageUrl.scheme());
    root.setHost(pageUrl.host());
    root.setPort(pageUrl.port());
    root.setPath(QStringLiteral("/"));
    return root;
}

Subscription *PageSaver::bestFeedFor(const QUrl &url) const
{
    Subscription *best{nullptr};
    int bestScore{-1};
    for (Subscription *feed : m_context->getFeeds()) {
        int score{-1};
        const int linkScore = UrlMatch::siteMatchScore(url, feed->link());
        if (linkScore >= 0) {
            score = linkScore + 1;
        } else if (UrlMatch::siteMatchScore(url, siteRoot(feed->url())) >= 0) {
            score = 0;
        }
        if (score > bestScore) {
            best = feed;
            bestScore = score;
        }
    }
    return best;
}

Subscription *PageSaver::feedByUrl(const QUrl &url) const
{
    const QString key = UrlMatch::normalizedUrlKey(url);
    if (key.isEmpty()) {
        return nullptr;
    }
    for (Subscription *feed : m_context->getFeeds()) {
        if (UrlMatch::normalizedUrlKey(feed->url()) == key) {
            return feed;
        }
    }
    return nullptr;
}

QFuture<PageSaver::Discovery> PageSaver::discover(const QUrl &siteUrl)
{
    auto promise = Future::makeSharedPromise<Discovery>();
    auto *feed = new ProvisionalSubscription(this);
    feed->setUrl(siteUrl);
    feed->update();
    whenNotUpdating(feed, m_timeout, this, [feed = QPointer(feed), promise, siteUrl] {
        Discovery result;
        if (feed && feed->status() == Feed::Idle && feed->unreadCount() > 0) {
            result.url = feed->url();
            result.name = feed->name().isEmpty() ? UrlMatch::normalizedHost(siteUrl) : feed->name();
            result.isWebPage = (feed->flags() & Subscription::IsWebPageFlag) != 0;
        }
        if (feed) {
            feed->cancelUpdates();
            feed->deleteLater();
        }
        Future::fulfill(promise, result);
    });
    return promise->future();
}

QFuture<PageSaver::Resolution> PageSaver::resolve(const QUrl &url)
{
    auto promise = Future::makeSharedPromise<Resolution>();
    auto articles = m_storage->getByUrl(url);
    Future::safeThen(articles, this, [this, url, promise](auto &articles) {
        Resolution resolution;
        const auto results = Future::safeResults(articles);
        if (!results.isEmpty()) {
            resolution.article = results.first();
            resolution.suggestedFeed = qobject_cast<Subscription *>(resolution.article->feed());
            Future::fulfill(promise, resolution);
            return;
        }

        resolution.suggestedFeed = bestFeedFor(url);
        if (resolution.suggestedFeed) {
            Future::fulfill(promise, resolution);
            return;
        }

        auto discovery = discover(siteRoot(url));
        Future::safeThen(discovery, this, [this, promise, partial = resolution](auto &discovery) {
            Resolution resolution = partial;
            const Discovery result = discovery.result();
            if (Subscription *existing = feedByUrl(result.url)) {
                // the site's feed is one we already have, the site link just didn't match
                resolution.suggestedFeed = existing;
            } else {
                resolution.discovered = result;
            }
            Future::fulfill(promise, resolution);
        });
    });
    return promise->future();
}

QFuture<PageSaver::Result> PageSaver::save(const Request &request)
{
    auto promise = Future::makeSharedPromise<Result>();
    if (!request.url.isValid() || request.url.isRelative()) {
        Future::fulfill(promise, Result{nullptr, false, InvalidUrl});
        return promise->future();
    }
    extractContent(request, [this, request, promise](const QString &content) {
        if (content.isEmpty()) {
            Future::fulfill(promise, Result{nullptr, false, ExtractionFailed});
            return;
        }
        storePage(request, content, [promise](const Result &result) {
            Future::fulfill(promise, result);
        });
    });
    return promise->future();
}

void PageSaver::extractContent(const Request &request, std::function<void(const QString &)> callback)
{
    ReadabilityResult *result = request.html.isEmpty() ? readability()->fetch(request.url) : readability()->parse(request.html, request.url);
    auto *guard = new QObject(this);
    QObject::connect(result, &ReadabilityResult::finished, guard, [guard, callback](const QString &content) {
        guard->deleteLater();
        callback(content.trimmed().isEmpty() ? QString() : content);
    });
    QObject::connect(result, &ReadabilityResult::error, guard, [guard, callback] {
        guard->deleteLater();
        callback(QString());
    });
}

void PageSaver::storePage(const Request &request, const QString &content, std::function<void(const Result &)> callback)
{
    auto existing = m_storage->getByUrl(request.url);
    Future::safeThen(existing, this, [this, request, content, callback](auto &existing) {
        const auto results = Future::safeResults(existing);
        if (!results.isEmpty()) {
            const ArticleRef &article = results.first();
            article->storeSavedContent(content);
            article->setStarred(true);
            callback(Result{article, true, NoError});
            return;
        }

        Subscription *feed{nullptr};
        switch (request.target) {
        case Request::SubscribeToFeed:
            if (Subscription *existingFeed = feedByUrl(request.subscribe.url)) {
                feed = existingFeed;
                break;
            }
            storeInSubscription(request, content, callback);
            return;

        case Request::ExistingFeed:
            feed = request.feed;
            if (feed == nullptr) {
                callback(Result{nullptr, false, FeedNotFound});
                return;
            }
            break;

        case Request::NoFeed:
            break;
        }

        SavedPage page;
        page.url = request.url;
        page.title = request.title.isEmpty() ? request.url.toString() : request.title;
        page.author = request.author.isEmpty() ? UrlMatch::normalizedHost(request.url) : request.author;
        page.content = content;
        auto stored = m_storage->storeSavedPage(feed, page);
        Future::safeThen(stored, this, [this, callback](auto &stored) {
            const auto results = Future::safeResults(stored);
            if (results.isEmpty()) {
                callback(Result{nullptr, false, StorageFailed});
            } else {
                callback(Result{results.first(), false, NoError});
            }
        });
    });
}

void PageSaver::storeInSubscription(const Request &request, const QString &content, std::function<void(const Result &)> callback)
{
    auto *provisional = new ProvisionalSubscription(this);
    provisional->setUrl(request.subscribe.url);
    provisional->setName(request.subscribe.name);

    auto *guard = new QObject(this);
    QObject::connect(provisional, &ProvisionalSubscription::saveFailed, guard, [this, guard, provisional, callback] {
        guard->deleteLater();
        provisional->deleteLater();
        callback(Result{nullptr, false, SubscribeFailed});
    });
    QObject::connect(provisional, &ProvisionalSubscription::targetFeedChanged, guard, [this, guard, provisional, request, content, callback] {
        guard->deleteLater();
        provisional->deleteLater();
        Subscription *feed = provisional->targetFeed();
        if (feed == nullptr) {
            callback(Result{nullptr, false, SubscribeFailed});
            return;
        }

        // Let the new subscription populate first, so that if the page is already in the
        // feed it is merged with the existing article instead of being added separately.
        feed->update();
        whenNotUpdating(feed, m_timeout, this, [this, feed = QPointer(feed), request, content, callback] {
            Request next = request;
            next.target = Request::ExistingFeed;
            next.feed = feed;
            storePage(next, content, callback);
        });
    });
    m_context->addFeed(provisional);
}
