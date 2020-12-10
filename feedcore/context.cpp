#include "context.h"

#include <QSortFilterProxyModel>
#include <QDebug>

#include "sqlitefeedstorage.h"
#include "updatescheduler.h"
#include "feedupdater.h"

namespace FeedCore {

struct Context::PrivData {
    Context *parent;
    std::unique_ptr<FeedStorage> storage;
    std::unique_ptr<UpdateScheduler> updateScheduler;

    PrivData(Context *parent);
    void configureUpdater();
};

Context::Context(QObject *parent)
    : QObject(parent),
      priv(std::make_unique<PrivData>(this))
{
    priv->configureUpdater();
}


Context::~Context() = default;

Context::PrivData::PrivData(Context *parent) :
    parent(parent),
    storage(std::make_unique<SqliteFeedStorage>()),
    updateScheduler(std::make_unique<UpdateScheduler>())
{ }

FeedQuery *Context::startFeedQuery()
{
    return priv->storage->getFeeds();
}

void Context::setRead(qint64 id, bool value)
{
    auto *q = priv->storage->updateItemRead(id, value);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        for (const auto &item : q->result()) {
            emit itemChanged(item);
            emit itemReadChanged(item);
        }
    });
}

void Context::setStarred(qint64 id, bool value)
{
    auto *q = priv->storage->updateItemStarred(id, !value);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        for (const auto &item : q->result()) {
            emit itemChanged(item);
            emit itemStarredChanged(item);
        }
    });
}

void Context::addFeed(const QUrl &url)
{
    qDebug() << "addFeed called";
    auto *q = priv->storage->storeFeed(url);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        for (const auto &feed : q->result()) {
            emit feedAdded(feed);
        }
    });
    priv->updateScheduler->schedule(q);
}

ItemQuery *Context::startQuery(const FeedRef &feedFilter, bool unreadFilter)
{
    return priv->storage->startItemQuery(feedFilter, unreadFilter);
}

LoadStatus Context::getFeedStatus(const FeedRef &feed)
{
    return priv->updateScheduler->getStatus(feed);
}

void Context::requestUpdate()
{
    priv->updateScheduler->updateAll();
}

void Context::requestUpdate(const FeedRef &feed)
{
    priv->updateScheduler->update(feed);
}

bool Context::updatesInProgress()
{
    return priv->updateScheduler->updatesInProgress();
}

void Context::slotFeedLoaded(FeedUpdater *updater, const Syndication::FeedPtr &content)
{
    qDebug() << "Got Feed " << content->title() << "\n";
    auto feed = updater->feed();
    auto *updateFeedQuery = priv->storage->updateFeed(feed, content);
    QObject::connect(updateFeedQuery, &FeedStorageOperation::finished, this, [this, updateFeedQuery]{
        for (const auto &feed : updateFeedQuery->result()) {
            emit feedNameChanged(feed, feed->name());
        }
    });

    const auto &items = content->items();
    for (const auto &item : items) {
        auto *q = priv->storage->storeItem(updater->feed(), item);
        QObject::connect(q, &FeedStorageOperation::finished, this, [this,q]{
            for (const auto &item : q->result()) {
                emit itemAdded(item);
            }
        });
    }
}

void Context::slotFeedStatusChanged(FeedUpdater *updater, LoadStatus status)
{
    emit feedStatusChanged(updater->feed(), status);
}

void Context::PrivData::configureUpdater() {
    QObject::connect(updateScheduler.get(), &UpdateScheduler::feedLoaded, parent, &Context::slotFeedLoaded);
    QObject::connect(updateScheduler.get(), &UpdateScheduler::feedStatusChanged, parent, &Context::slotFeedStatusChanged);
    auto *queryFeeds = storage->getFeeds();
    updateScheduler->schedule(queryFeeds);
    updateScheduler->start();
}

}
