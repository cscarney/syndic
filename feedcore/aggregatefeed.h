/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "feed.h"
#include <QSet>

namespace FeedCore
{
/**
 * A Feed implementation that combines all articles from multiple feeds.
 */
class AggregateFeed : public FeedCore::Feed
{
    Q_OBJECT
public:
    explicit AggregateFeed(QObject *parent = nullptr);
    FeedCore::Feed::Updater *updater() override;
    QFuture<FeedCore::ArticleRef> getArticles(bool unreadOnly) override;

protected:
    void addFeed(Feed *feed);
    void removeFeed(Feed *feed);
    void setIdleStatus(FeedCore::Feed::LoadStatus status);

private:
    class Updater;
    QSet<Feed *> m_feeds;
    QSet<Feed *> m_active;
    Updater *m_updater{nullptr};
    LoadStatus m_idleStatus{Feed::Idle};
    void onUnreadCountChanged(int delta);
    void onArticleAdded(const ArticleRef &article);
    void setFeedActive(FeedCore::Feed *feed, bool active);
    void syncFeedStatus(FeedCore::Feed *sender);
};
}
