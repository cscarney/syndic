/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "articleref.h"
#include "future.h"
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include <Syndication/Item>

namespace FeedCore
{
class Feed;
class Subscription;

/**
 * A web page that the user saved directly (e.g. from a browser), rather
 * than one that arrived through a feed update.
 */
struct SavedPage {
    QUrl url;
    QString title;
    QString author;
    QString content;
};

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
     */
    virtual QFuture<Subscription *> getFeeds() = 0;

    /**
     * Store a new subscription using the configuration in /feed/, returning the stored subscription.
     */
    virtual QFuture<Subscription *> storeFeed(Subscription *feed) = 0;

    /**
     * Find stored articles whose URL is equivalent to /url/ (see UrlMatch::normalizedUrlKey),
     * most recent first.
     */
    virtual QFuture<ArticleRef> getByUrl(const QUrl &url) = 0;

    /**
     * Store a saved page as a new read, starred article.
     *
     * If /feed/ is null, the article is not associated with any subscription and is
     * never expired. Otherwise, the article belongs to /feed/ and will be merged with
     * an article with an equivalent URL if one later arrives in that feed.
     *
     * The page content is stored as both the feed content and the readable content.
     */
    virtual QFuture<ArticleRef> storeSavedPage(Subscription *feed, const SavedPage &page) = 0;
};
}
