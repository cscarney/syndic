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
    virtual QFuture<Feed *> getFeeds() = 0;
    virtual QFuture<Feed *> storeFeed(Feed *feed) = 0;
};
}
