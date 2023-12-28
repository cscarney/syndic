/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "aggregatefeed.h"
#include "article.h"
using namespace FeedCore;

class AggregateFeed::Updater : public Feed::Updater
{
public:
    Updater(AggregateFeed *feed, QObject *parent)
        : Feed::Updater(feed, parent)
    {
    }

    void run() final
    {
        if (auto *af = qobject_cast<AggregateFeed *>(feed())) {
            // TODO need to pass the pending update timestamp
            for (auto *subfeed : std::as_const(af->m_feeds)) {
                subfeed->updater()->start(updateStartTime());
            }
            finish();
        }
    }

    void abort() final
    {
        if (auto *af = qobject_cast<AggregateFeed *>(feed())) {
            for (auto *subfeed : std::as_const(af->m_active)) {
                subfeed->updater()->abort();
            }
        }
    }
};

AggregateFeed::AggregateFeed(QObject *parent)
    : Feed(parent)
    , m_updater{new AggregateFeed::Updater(this, this)}
{
}

Feed::Updater *AggregateFeed::updater()
{
    return m_updater;
}

QFuture<ArticleRef> AggregateFeed::getArticles(bool unreadOnly)
{
    QList<QFuture<ArticleRef>> sourceOps;
    for (auto *f : std::as_const(m_feeds)) {
        sourceOps.append(f->getArticles(unreadOnly));
    }
    auto aggregateOp = QtFuture::whenAll(sourceOps.begin(), sourceOps.end());

    QPromise<ArticleRef> p;
    auto result = p.future();
    aggregateOp.then([p = std::move(p)](const QList<QFuture<ArticleRef>> &results) mutable {
        p.start();
        for (const auto &resultFuture : results) {
            const auto result = Future::safeResults(resultFuture);
            for (const auto &eachArticle : result) {
                p.addResult(eachArticle);
            }
        }
        p.finish();
    });
    return result;
}

void AggregateFeed::addFeed(Feed *feed)
{
    incrementUnreadCount(feed->unreadCount());
    syncFeedStatus(feed);
    QObject::connect(feed, &Feed::articleAdded, this, &AggregateFeed::onArticleAdded);
    QObject::connect(feed, &Feed::unreadCountChanged, this, &AggregateFeed::incrementUnreadCount);
    QObject::connect(feed, &Feed::statusChanged, this, [this, feed] {
        syncFeedStatus(feed);
    });
    QObject::connect(feed, &Feed::destroyed, this, [this, feed] {
        removeFeed(feed);
    });
    m_feeds << feed;
    emit reset();
}

void AggregateFeed::removeFeed(Feed *feed)
{
    setFeedActive(feed, false);
    m_feeds.remove(feed);
}

void AggregateFeed::onArticleAdded(const ArticleRef &article)
{
    emit articleAdded(article);
}

void AggregateFeed::setFeedActive(Feed *feed, bool active)
{
    if (active) {
        m_active.insert(feed);
    } else {
        m_active.remove(feed);
    }
    setStatus(m_active.isEmpty() ? m_idleStatus : LoadStatus::Updating);
}

void AggregateFeed::syncFeedStatus(Feed *sender)
{
    setFeedActive(sender, sender->status() == LoadStatus::Updating);
}

void AggregateFeed::setIdleStatus(LoadStatus status)
{
    // the status to use when no feeds are updating.
    // this supports the case where e.g. the aggregate
    // feed itself is loading or in error.

    if (m_idleStatus == status) {
        return;
    }
    m_idleStatus = status;
    if (m_active.isEmpty()) {
        setStatus(status);
    }
}
