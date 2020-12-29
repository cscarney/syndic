#include "context.h"
#include <QSortFilterProxyModel>
#include <QDebug>
#include "storage.h"
#include "scheduler.h"
#include "feedref.h"

namespace FeedCore {

struct Context::PrivData {
    Context *parent;
    Storage *storage;
    Scheduler *updateScheduler;

    PrivData(Storage *storage, Context *parent);
};

Context::Context(Storage *storage, QObject *parent)
    : QObject(parent),
      priv(std::make_unique<PrivData>(storage, this))
{
    Future<FeedRef> *queryFeeds { priv->storage->getFeeds() };
    priv->updateScheduler->schedule(queryFeeds);
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

Future<FeedRef> *Context::startFeedQuery()
{
    return priv->storage->getFeeds();
}

void Context::addFeed(const QUrl &url)
{
    qDebug() << "addFeed called";
    Future<FeedRef> *q { priv->storage->storeFeed(url) };
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        for (const auto &feed : q->result()) {
            emit feedAdded(feed);
        }
    });
    priv->updateScheduler->schedule(q);
}

Future<ArticleRef> *Context::startQuery(bool unreadFilter)
{
    if (unreadFilter) {
        return priv->storage->getUnread();
    }
    return priv->storage->getAll();
}

void Context::requestUpdate()
{
    priv->updateScheduler->updateAll();
}

bool Context::updatesInProgress()
{
    return priv->updateScheduler->updatesInProgress();
}

}
