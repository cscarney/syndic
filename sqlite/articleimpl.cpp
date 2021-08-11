/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "sqlite/articleimpl.h"
#include <QSqlQuery>
#include <QVariant>
#include "sqlite/storageimpl.h"
#include "sqlite/feedimpl.h"

using namespace FeedCore;
using namespace Sqlite;

ArticleImpl::ArticleImpl(qint64 id, StorageImpl *storage, FeedImpl *feed, const ItemQuery &q):
    Article(feed, nullptr),
    m_id{ id }
{
    updateFromQuery(q);
    QObject::connect(this, &Article::readStatusChanged, storage, [this, storage]{
        storage->onArticleReadChanged(this);
    });
    QObject::connect(this, &Article::readStatusChanged, feed, [this, feed]{
        feed->onArticleReadChanged(this);
    });
    QObject::connect(this, &Article::starredChanged, storage, [this, storage]{
        storage->onArticleStarredChanged(this);
    });
}

qint64 ArticleImpl::id() const
{
    return m_id;
}

void ArticleImpl::updateFromQuery(const ItemQuery &q)
{
    Article::setTitle(q.headline());
    Article::setAuthor(q.author());
    Article::setDate(q.date());
    Article::setUrl(q.url());
    m_content = q.content();
    Article::setRead(q.isRead());
    Article::setStarred(q.isStarred());
}

void ArticleImpl::requestContent()
{
    emit gotContent(m_content);
}
