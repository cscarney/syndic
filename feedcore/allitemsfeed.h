/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "feed.h"
#include <QSet>

namespace FeedCore
{
class Context;

/**
 * A Feed implementation that combines all articles from a context.
 */
class AllItemsFeed : public Feed
{
    Q_OBJECT
public:
    AllItemsFeed(Context *context, const QString &name, QObject *parent = nullptr);
    Future<ArticleRef> *getArticles(bool unreadFilter) final;
    Updater *updater() final;

private:
    Context *m_context{nullptr};
    Updater *m_updater{nullptr};
    QSet<Feed *> m_active;
    void addFeed(Feed *feed);
    void onUnreadCountChanged(int delta);
    void onArticleAdded(const ArticleRef &article);
    void setFeedActive(FeedCore::Feed *feed, bool active);
    void syncFeedStatus(FeedCore::Feed *sender);
    void onFeedDestroyed(FeedCore::Feed *sender);
};
}
