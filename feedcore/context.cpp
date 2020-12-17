#include "context.h"

#include <QSortFilterProxyModel>
#include <QDebug>

#include "sqlitefeedstorage.h"
#include "updatescheduler.h"
#include "feedupdater.h"
#include "feed.h"

namespace FeedCore {

struct Context::PrivData {
    Context *parent;
    std::unique_ptr<FeedStorage> storage;
    std::unique_ptr<UpdateScheduler> updateScheduler;

    PrivData(Context *parent);
};

Context::Context(QObject *parent)
    : QObject(parent),
      priv(std::make_unique<PrivData>(this))
{
    FeedQuery *queryFeeds { priv->storage->getFeeds() };
    priv->updateScheduler->schedule(queryFeeds);
    priv->updateScheduler->start();
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

void Context::addFeed(const QUrl &url)
{
    qDebug() << "addFeed called";
    FeedQuery *q { priv->storage->storeFeed(url) };
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        for (const auto &feed : q->result()) {
            emit feedAdded(feed);
        }
    });
    priv->updateScheduler->schedule(q);
}

ItemQuery *Context::startQuery(bool unreadFilter)
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

void Context::requestUpdate(const FeedRef &feed)
{
    priv->updateScheduler->update(feed);
}

bool Context::updatesInProgress()
{
    return priv->updateScheduler->updatesInProgress();
}

}
