/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "sqlite/feedimpl.h"
#include "article.h"
#include "articleimpl.h"
#include "articleref.h"
#include "sqlite/feedquery.h"
#include "sqlite/storageimpl.h"
#include <QSqlQuery>
#include <QVariant>
#include <Syndication/Image>

using namespace FeedCore;
using namespace SqliteStorage;

FeedImpl::FeedImpl(qint64 feedId, StorageImpl *storage)
    : UpdatableFeed(storage)
    , m_id{feedId}
    , m_storage{storage}
{
    storage->listenForChanges(this);
}

void FeedImpl::unpackUpdateInterval(qint64 updateInterval)
{
    if (updateInterval == 0) {
        setUpdateMode(InheritUpdateMode);
    } else if (updateInterval < 0) {
        setUpdateMode(DisableUpdateMode);
    } else {
        setUpdateMode(OverrideUpdateMode);
        setUpdateInterval(updateInterval);
    }
}

void FeedImpl::updateFromQuery(const FeedQuery &query)
{
    setName(query.displayName());
    setCategory(query.category());
    setUrl(query.url());
    setLink(query.link());
    setIcon(query.icon());
    setUnreadCount(query.unreadCount());
    setLastUpdate(query.lastUpdate());
    unpackUpdateInterval(query.updateInterval());
}

Future<ArticleRef> *FeedImpl::getArticles(bool unreadFilter)
{
    if (unreadFilter) {
        return m_storage->getUnreadByFeed(this);
    }
    return m_storage->getByFeed(this);
}

void FeedImpl::updateSourceArticle(const Syndication::ItemPtr &article)
{
    auto *q = m_storage->storeArticle(this, article);
    QObject::connect(q, &BaseFuture::finished, this, [this, q] {
        for (const auto &item : q->result()) {
            if (!item->isRead()) {
                incrementUnreadCount();
            }
            emit articleAdded(item);
        }
    });
}

void FeedImpl::expire(const QDateTime &olderThan)
{
    m_storage->expire(this, olderThan);
}

void FeedImpl::onArticleReadChanged(ArticleImpl *article)
{
    incrementUnreadCount(article->isRead() ? -1 : 1);
}

qint64 FeedImpl::id() const
{
    return m_id;
}
