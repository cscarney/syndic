/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "context.h"

#include <QSet>
#include "storage.h"
#include "scheduler.h"
#include "feed.h"
#include "future.h"
#include <QNetworkConfigurationManager>

namespace FeedCore {

struct Context::PrivData {
    Context *parent;
    Storage *storage;
    QSet<Feed*> feeds;
    qint64 updateInterval{ 0 };
    qint64 expireAge { 0 };
    Scheduler *updateScheduler;
    QNetworkConfigurationManager ncm;

    PrivData(Storage *storage, Context *parent);
    void configureUpdates(Feed *feed, const QDateTime &timestamp=QDateTime::currentDateTime()) const;
};

Context::Context(Storage *storage, QObject *parent)
    : QObject(parent),
      d{ std::make_unique<PrivData>(storage, this) }
{
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationAdded, d->updateScheduler, &Scheduler::clearErrors);
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationChanged, d->updateScheduler, &Scheduler::clearErrors);

    Future<Feed*> *getFeeds { d->storage->getFeeds() };
    QObject::connect(getFeeds, &BaseFuture::finished, this, [this, getFeeds] {
        populateFeeds(getFeeds->result());
    });
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

void Context::PrivData::configureUpdates(Feed *feed, const QDateTime &timestamp) const
{
    auto updateMode{feed->updateMode()};
    if (updateMode==Feed::DefaultUpdateMode) {
        feed->setUpdateInterval(updateInterval);
    }
    if (updateMode==Feed::ManualUpdateMode) {
        updateScheduler->unschedule(feed);
    } else {
        updateScheduler->schedule(feed, timestamp);
    }
}

const QSet<Feed*> &Context::getFeeds()
{
    return d->feeds;
}

void Context::addFeed(Feed *feed)
{
    Future<Feed*> *q { d->storage->storeFeed(feed) };
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        populateFeeds(q->result());
    });
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
    const auto &timestamp = QDateTime::currentDateTime();
    const auto &feeds = d->feeds;
    for (Feed *const entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

void Context::abortUpdates()
{
    const auto &feeds = d->feeds;
    for(Feed *const entry : feeds) {
        entry->updater()->abort();
    }
}

qint64 Context::defaultUpdateInterval()
{
    return d->updateInterval;
}

void Context::setDefaultUpdateInterval(qint64 defaultUpdateInterval)
{
    if (d->updateInterval == defaultUpdateInterval) {
        return;
    }
    d->updateInterval = defaultUpdateInterval;
    for (Feed *feed : qAsConst(d->feeds)) {
        if (feed->updateMode() == Feed::DefaultUpdateMode) {
            feed->setUpdateInterval(defaultUpdateInterval);
        }
    }
    emit defaultUpdateIntervalChanged();
}

qint64 Context::expireAge()
{
    return d->expireAge;
}

void Context::setExpireAge(qint64 expireAge)
{
    if (d->expireAge == expireAge) {
        return;
    }
    d->expireAge = expireAge;
    for (Feed *feed : qAsConst(d->feeds)) {
        feed->setExpireAge(expireAge);
    }
    emit expireAgeChanged();
}

void Context::populateFeeds(const QVector<Feed*> &feeds)
{
    const QDateTime timestamp = QDateTime::currentDateTime();
    for(const auto &feed : feeds) {
        d->feeds.insert(feed);
        feed->setExpireAge(d->expireAge);
        d->configureUpdates(feed, timestamp);
        QObject::connect(feed, &Feed::updateModeChanged, this, [this, feed]{
            d->configureUpdates(feed);
        });
        emit feedAdded(feed);
    }
}

}
