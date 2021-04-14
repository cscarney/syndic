#include "context.h"
#include "storage.h"
#include "scheduler.h"
#include "feed.h"
#include "cachednetworkaccessmanager.h"

namespace FeedCore {

struct Context::PrivData {
    Context *parent;
    Storage *storage;
    Scheduler *updateScheduler;
    CachedNetworkAccessManager nam;

    PrivData(Storage *storage, Context *parent);
};

Context::Context(Storage *storage, QObject *parent)
    : QObject(parent),
      priv{ std::make_unique<PrivData>(storage, this) }
{
    Future<Feed*> *getFeeds { priv->storage->getFeeds() };
    priv->updateScheduler->schedule(getFeeds);
    priv->updateScheduler->start();
}


Context::~Context() = default;

Context::PrivData::PrivData(Storage *storage, Context *parent) :
    parent(parent),
    storage(storage),
    updateScheduler(new Scheduler(parent))
{
    storage->setParent(parent);
}

Future<Feed*> *Context::getFeeds()
{
    return priv->storage->getFeeds();
}

void Context::addFeed(Feed *feed)
{
    Future<Feed*> *q { priv->storage->storeFeed(feed) };
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        for (const auto &feed : q->result()) {
            emit feedAdded(feed);
        }
    });
    priv->updateScheduler->schedule(q);
}

Future<ArticleRef> *Context::getArticles(bool unreadFilter)
{
    if (unreadFilter) {
        return priv->storage->getUnread();
    }
    return priv->storage->getAll();
}

Future<ArticleRef> *Context::getStarred()
{
    return priv->storage->getStarred();
}

void Context::requestUpdate()
{
    priv->updateScheduler->updateAll();
}

void Context::abortUpdates()
{
    priv->updateScheduler->abortAll();
}

qint64 Context::defaultUpdateInterval()
{
    return priv->updateScheduler->updateInterval();
}

void Context::setDefaultUpdateInterval(qint64 defaultUpdateInterval)
{
    if (this->defaultUpdateInterval() == defaultUpdateInterval) {
        return;
    }
    priv->updateScheduler->setUpdateInterval(defaultUpdateInterval);
    emit defaultUpdateIntervalChanged();
}

QNetworkAccessManager *Context::networkAccessManager() const
{
    return &priv->nam;
}

}
