#include "sqlite/feedimpl.h"
#include <QVariant>
#include <QSqlQuery>
#include "xmlupdater.h"
#include "sqlite/storageimpl.h"
#include "article.h"
#include "articleref.h"
using namespace FeedCore;
using namespace Sqlite;


FeedImpl::FeedImpl::FeedImpl(StorageImpl *storage, qint64 feedId):
    m_storage(storage),
    m_id(feedId),
    m_updater(new XMLUpdater(this, 3600, 0, this))
{
}

void FeedImpl::updateFromQuery(const QSqlQuery &query)
{
    populateName(query.value(3).toString());
    populateUrl(query.value(4).toString());
    populateUnreadCount(query.value(5).toInt());
}

void FeedImpl::populateNew(const QUrl &url, const QString &name)
{
    populateName(name);
    populateUrl(url);
    populateUnreadCount(0);
}

Future<ArticleRef> *FeedImpl::startItemQuery(bool unreadFilter)
{
    if (unreadFilter) {
        return m_storage->getUnreadByFeed(this);
    } else {
        return m_storage->getByFeed(this);
    }
}

void FeedImpl::updateFromSource(const Syndication::FeedPtr &source)
{
    setName(source->title());
    const auto &items = source->items();
    for (const auto &item : items) {
        auto *q = m_storage->storeItem(this, item);
        QObject::connect(q, &BaseFuture::finished, this, [this, q]{
            for (const auto &item : q->result()) {
                if (!item->isRead()){
                    incrementUnreadCount();
                }
                emit itemAdded(item);
            }
        });
    }
}

Updater *FeedImpl::updater()
{
    return m_updater;
}

void FeedImpl::setName(const QString &name)
{
    if (populateName(name)) {
        m_storage->updateFeedMetadata(this);
    }
}

void FeedImpl::setItemRead(ArticleImpl *item, bool isRead)
{
    auto *q = m_storage->updateItemRead(item, isRead);
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

QSharedPointer<FeedImpl> FeedImpl::forId(StorageImpl *storage, qint64 feedId)
{
    static QHash<qint64, QWeakPointer<FeedImpl>> instances;
    auto &instance = instances[feedId];
    if (instance.isNull()) {
        auto newFeed = QSharedPointer<FeedImpl>(new FeedImpl(storage, feedId));
        instance = newFeed;
        return newFeed;
    }
    return instance.toStrongRef();
}

QSharedPointer<FeedImpl> FeedImpl::fromQuery(StorageImpl *storage, const QSqlQuery &q)
{
    qint64 id = q.value(0).toLongLong();
    const auto &result = forId(storage, id);
    result->updateFromQuery(q);
    return result;
}

qint64 FeedImpl::id() const
{
    return m_id;
}
