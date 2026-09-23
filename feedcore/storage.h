/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "articleref.h"
#include "future.h"
#include <QObject>
#include <Syndication/Feed>
#include <Syndication/Item>

namespace FeedCore
{
class Feed;
class Subscription;

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent = nullptr)
        : QObject(parent){};
    virtual QFuture<ArticleRef> getAll() = 0;
    virtual QFuture<ArticleRef> getUnread() = 0;
    virtual QFuture<ArticleRef> getStarred() = 0;
    virtual QFuture<ArticleRef> getSearchResults(const QString &search) = 0;
    virtual QFuture<ArticleRef> getHighlights(size_t limit) = 0;
    /**
     * Load all stored feeds.
     *
     * Backends that manage subscriptions locally should return Subscription
     * instances.  Backends where subscriptions are managed elsewhere (e.g. by a
     * server) may return plain Feed instances, which will not be scheduled
     * or editable.
     */
    virtual QFuture<Feed *> getFeeds() = 0;

    /**
     * Store a new subscription using the configuration in /feed/, returning the stored subscription.
     *
     * Backends that do not support adding subscriptions should return an empty result.
     */
    virtual QFuture<Subscription *> storeFeed(Subscription *feed) = 0;
};
}
