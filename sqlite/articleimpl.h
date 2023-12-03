/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "article.h"
#include "factory.h"
class QSqlQuery;

namespace SqliteStorage
{
class FeedImpl;
class StorageImpl;
class ItemQuery;

class ArticleImpl : public FeedCore::Article
{
    Q_OBJECT
public:
    ArticleImpl(qint64 id, StorageImpl *storage, FeedImpl *feed, const ItemQuery &q);
    qint64 id() const;
    void updateFromQuery(const ItemQuery &q);
    void requestContent() final;
    QFuture<QString> getCachedReadableContent() final;
    void cacheReadableContent(const QString &readableContent) final;

private:
    qint64 m_id;
    QPointer<StorageImpl> m_storage;
};
}
