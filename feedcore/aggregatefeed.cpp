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
                subfeed->updater()->start();
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

FeedCore::Future<ArticleRef> *AggregateFeed::getArticles(bool unreadOnly)
{
    return UnionFuture<ArticleRef>::create([this, unreadOnly](auto *op) {
        for (auto *f : std::as_const(m_feeds)) {
            op->addFuture(f->getArticles(unreadOnly));
        }
    });
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
    setStatus(m_active.isEmpty() ? LoadStatus::Idle : LoadStatus::Updating);
}

void AggregateFeed::syncFeedStatus(Feed *sender)
{
    setFeedActive(sender, sender->status() == LoadStatus::Updating);
}
