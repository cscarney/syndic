/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "context.h"
#include "storage.h"
#include "scheduler.h"
#include "feed.h"

namespace FeedCore {

struct Context::PrivData {
    Context *parent;
    Storage *storage;
    Scheduler *updateScheduler;

    PrivData(Storage *storage, Context *parent);
};

Context::Context(Storage *storage, QObject *parent)
    : QObject(parent),
      d{ std::make_unique<PrivData>(storage, this) }
{
    Future<Feed*> *getFeeds { d->storage->getFeeds() };
    d->updateScheduler->schedule(getFeeds);
    d->updateScheduler->start();
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
    return d->storage->getFeeds();
}

void Context::addFeed(Feed *feed)
{
    Future<Feed*> *q { d->storage->storeFeed(feed) };
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        for (const auto &feed : q->result()) {
            emit feedAdded(feed);
        }
    });
    d->updateScheduler->schedule(q);
}

Future<ArticleRef> *Context::getArticles(bool unreadFilter)
{
    if (unreadFilter) {
        return d->storage->getUnread();
    }
    return d->storage->getAll();
}

Future<ArticleRef> *Context::getStarred()
{
    return d->storage->getStarred();
}

void Context::requestUpdate()
{
    d->updateScheduler->updateAll();
}

void Context::abortUpdates()
{
    d->updateScheduler->abortAll();
}

qint64 Context::defaultUpdateInterval()
{
    return d->updateScheduler->updateInterval();
}

void Context::setDefaultUpdateInterval(qint64 defaultUpdateInterval)
{
    if (this->defaultUpdateInterval() == defaultUpdateInterval) {
        return;
    }
    d->updateScheduler->setUpdateInterval(defaultUpdateInterval);
    emit defaultUpdateIntervalChanged();
}

qint64 Context::expireAge()
{
    return d->updateScheduler->expireAge();
}

void Context::setExpireAge(qint64 expireAge)
{
    if (this->expireAge() == expireAge) {
        return;
    }
    d->updateScheduler->setExpireAge(expireAge);
    emit expireAgeChanged();
}

}
