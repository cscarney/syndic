#include "sqlite/feedimpl.h"
#include <QVariant>
#include <QSqlQuery>
#include "xmlupdater.h"
#include "sqlite/storageimpl.h"
#include "article.h"
#include "articleref.h"
#include "sqlite/feedquery.h"
using namespace FeedCore;
using namespace Sqlite;


FeedImpl::FeedImpl::FeedImpl(qint64 feedId, StorageImpl *storage):
    m_id{feedId},
    m_storage{storage},
    m_updater{new XMLUpdater(this, this)}
{
    storage->listenForChanges(this);
}

static void unpackUpdateInterval(Updater *updater, qint64 updateInterval)
{
    if (updateInterval == 0) {
        updater->setUpdateMode(Updater::DefaultUpdateMode);
    } else if (updateInterval < 0) {
        updater->setUpdateMode(Updater::MaunualUpdateMode);
    } else {
        updater->setUpdateInterval(updateInterval);
    }
}

void FeedImpl::updateFromQuery(const FeedQuery &query)
{
    Feed::setName(query.displayName());
    Feed::setUrl(query.url());
    Feed::setUnreadCount(query.unreadCount());

    m_updater->setLastUpdate(query.lastUpdate());
    unpackUpdateInterval(m_updater, query.updateInterval());
}

Future<ArticleRef> *FeedImpl::getArticles(bool unreadFilter)
{
    if (unreadFilter) {
        return m_storage->getUnreadByFeed(this);
    } else {
        return m_storage->getByFeed(this);
    }
}

void FeedImpl::updateFromSource(const Syndication::FeedPtr &source)
{
    if (name().isEmpty()) {
        setName(source->title());
    }
    const auto &items = source->items();
    for (const auto &item : items) {
        auto *q = m_storage->storeArticle(this, item);
        QObject::connect(q, &BaseFuture::finished, this, [this, q]{
            for (const auto &item : q->result()) {
                if (!item->isRead()){
                    incrementUnreadCount();
                }
                emit articleAdded(item);
            }
        });
    }
}

Updater *FeedImpl::updater()
{
    return m_updater;
}

bool FeedImpl::setName(const QString &name)
{
    const bool changed { Feed::setName(name) };
    if (changed) {
        m_storage->updateFeedMetadata(this);
    }
    return changed;
}

void FeedImpl::setRead(ArticleImpl *article, bool isRead)
{
    auto *q = m_storage->updateArticleRead(article, isRead);
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        const auto &changedItems = q->result();
        for (const auto &changedItem : changedItems) {
            if (changedItem->isRead()) {
                decrementUnreadCount();
            } else {
                incrementUnreadCount();
            }
        }
    });
}

qint64 FeedImpl::id() const
{
    return m_id;
}
