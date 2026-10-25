/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "sqlite/articleimpl.h"
#include "sqlite/feedimpl.h"
#include "sqlite/storageimpl.h"
#include <QSqlQuery>
#include <QVariant>

using namespace FeedCore;
using namespace SqliteStorage;

ArticleImpl::ArticleImpl(qint64 id, StorageImpl *storage, FeedCore::Feed *feed, const ItemQuery &q)
    : Article(feed, nullptr)
    , m_id{id}
    , m_storage(storage)
{
    updateFromQuery(q);
    QObject::connect(this, &Article::readStatusChanged, storage, [this, storage] {
        storage->onArticleReadChanged(this);
    });
    if (auto *feedImpl = qobject_cast<FeedImpl *>(feed)) {
        QObject::connect(this, &Article::readStatusChanged, feedImpl, [this, feedImpl] {
            feedImpl->onArticleReadChanged(this);
        });
    }
    QObject::connect(this, &Article::starredChanged, storage, [this, storage] {
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
    Article::setRead(q.isRead());
    Article::setStarred(q.isStarred());
    Article::setPreferReadable(q.preferReadable());
}

void ArticleImpl::requestContent()
{
    if (m_storage.isNull()) {
        return;
    }
    QFuture<QString> fut = m_storage->getContent(this);
    Future::safeThen(fut, this, [this](auto &fut) {
        emit gotContent(fut.result());
    });
}

QFuture<QString> ArticleImpl::getCachedReadableContent()
{
    return m_storage->getReadableContent(this);
}

void ArticleImpl::cacheReadableContent(const QString &readableContent)
{
    m_storage->cacheReadableContent(this, readableContent);
}

void ArticleImpl::storeSavedContent(const QString &readableContent)
{
    m_storage->storeSavedContent(this, readableContent);
    Article::setPreferReadable(true);
}
