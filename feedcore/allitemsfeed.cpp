/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "allitemsfeed.h"
#include "article.h"
#include "context.h"
using namespace FeedCore;

namespace
{
class AllUpdater : public Feed::Updater
{
public:
    AllUpdater(AllItemsFeed *feed, Context *context, QObject *parent)
        : Feed::Updater(feed, parent)
        , m_context{context}
    {
    }

    void run() final
    {
        m_context->requestUpdate();
        finish();
    }

    void abort() final
    {
        m_context->abortUpdates();
    }

private:
    Context *m_context;
};
}

AllItemsFeed::AllItemsFeed(Context *context, const QString &name, QObject *parent)
    : Feed(parent)
    , m_context{context}
    , m_updater{new AllUpdater(this, context, this)}
{
    setName(name);
    for (const auto &feed : context->getFeeds()) {
        addFeed(feed);
    }
    QObject::connect(context, &Context::feedAdded, this, &AllItemsFeed::addFeed);
}

Future<ArticleRef> *AllItemsFeed::getArticles(bool unreadFilter)
{
    return m_context->getArticles(unreadFilter);
}

Feed::Updater *AllItemsFeed::updater()
{
    return m_updater;
}

void AllItemsFeed::addFeed(Feed *feed)
{
    incrementUnreadCount(feed->unreadCount());
    syncFeedStatus(feed);
    QObject::connect(feed, &Feed::articleAdded, this, &AllItemsFeed::onArticleAdded);
    QObject::connect(feed, &Feed::unreadCountChanged, this, &AllItemsFeed::incrementUnreadCount);
    QObject::connect(feed, &Feed::statusChanged, this, [this, feed] {
        syncFeedStatus(feed);
    });
    QObject::connect(feed, &Feed::destroyed, this, [this, feed] {
        onFeedDestroyed(feed);
    });
    emit reset();
}

void AllItemsFeed::onArticleAdded(const ArticleRef &article)
{
    emit articleAdded(article);
}

void AllItemsFeed::setFeedActive(Feed *feed, bool active)
{
    if (active) {
        m_active.insert(feed);
    } else {
        m_active.remove(feed);
    }
    setStatus(m_active.isEmpty() ? LoadStatus::Idle : LoadStatus::Updating);
}

void AllItemsFeed::syncFeedStatus(Feed *sender)
{
    setFeedActive(sender, sender->status() == LoadStatus::Updating);
}

void AllItemsFeed::onFeedDestroyed(Feed *sender)
{
    setFeedActive(sender, false);
}
