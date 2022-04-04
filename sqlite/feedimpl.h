/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SQLITE_FEEDIMPL_H
#define SQLITE_FEEDIMPL_H
#include "factory.h"
#include "updatablefeed.h"
class QSqlQuery;
namespace FeedCore
{
class XMLUpdater;
}

namespace SqliteStorage
{
class StorageImpl;
class ArticleImpl;
class FeedQuery;

class FeedImpl : public FeedCore::UpdatableFeed
{
    Q_OBJECT
public:
    qint64 id() const;
    void updateFromQuery(const FeedQuery &query);
    FeedCore::Future<FeedCore::ArticleRef> *getArticles(bool unreadFilter) final;
    bool editable() final
    {
        return true;
    }
    void onArticleReadChanged(ArticleImpl *article);

private:
    FeedImpl(qint64 feedId, StorageImpl *storage);
    qint64 m_id{0};
    StorageImpl *m_storage{nullptr};
    void unpackUpdateInterval(qint64 updateInterval);
    void unpackExpireAge(qint64 expireAge);
    void updateSourceArticle(const Syndication::ItemPtr &article) final;
    void expire(const QDateTime &olderThan) final;
    friend FeedCore::ObjectFactory<qint64, FeedImpl>;
};
}
#endif // SQLITE_FEEDIMPL_H
