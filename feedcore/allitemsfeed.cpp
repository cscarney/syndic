/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "allitemsfeed.h"
#include "context.h"
#include "updater.h"
#include "article.h"
namespace FeedCore {

namespace {
    class AllUpdater : public FeedCore::Updater {
    public:
        AllUpdater(AllItemsFeed *feed, Context *context, QObject *parent):
            FeedCore::Updater(feed, parent),
            m_context { context }
        {}

        void run() final {
            m_context->requestUpdate();
            finish();
        }

        void abort() final {
            m_context->abortUpdates();
        }

    private:
        Context *m_context;
    };
}

AllItemsFeed::AllItemsFeed(Context *context, const QString &name, QObject *parent) :
    Feed(parent),
    m_context{ context },
    m_updater{ new AllUpdater(this, context, this) }
{
    Future<Feed*> *q { context->getFeeds() };
    setName(name);
    QObject::connect(q, &BaseFuture::finished, this,
                     [this, q]{ onGetFeedsFinished(q); });
    QObject::connect(context, &Context::feedAdded, this, &AllItemsFeed::addFeed);
}

Future<ArticleRef> *AllItemsFeed::getArticles(bool unreadFilter)
{
    return m_context->getArticles(unreadFilter);
}

Updater *AllItemsFeed::updater()
{
    return m_updater;
}

void AllItemsFeed::addFeed(Feed *feed)
{
    incrementUnreadCount(feed->unreadCount());
    syncFeedStatus(feed);
    QObject::connect(feed, &Feed::articleAdded, this, &AllItemsFeed::onArticleAdded);
    QObject::connect(feed, &Feed::unreadCountChanged, this, &AllItemsFeed::incrementUnreadCount);
    QObject::connect(feed, &Feed::statusChanged, this, [this, feed]{ syncFeedStatus(feed); });
}

void AllItemsFeed::onGetFeedsFinished(Future<Feed*> *sender)
{
    for (const auto &feed : sender->result()){
        addFeed(feed);
    }
}

void AllItemsFeed::onArticleAdded(const ArticleRef &article)
{
    emit articleAdded(article);
}

void AllItemsFeed::syncFeedStatus(Feed *sender)
{
    if (sender->status() == LoadStatus::Updating) {
        m_active.insert(sender);
    } else {
        m_active.remove(sender);
    }
    setStatus(m_active.isEmpty() ? LoadStatus::Idle : LoadStatus::Updating);
}

}
