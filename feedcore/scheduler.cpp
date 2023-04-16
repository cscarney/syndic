/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "scheduler.h"
#include "feed.h"
#include <QSet>

namespace FeedCore
{
struct Scheduler::PrivData {
    QList<Feed *> schedule;
    QTimer timer;
};

Scheduler::Scheduler(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<PrivData>())
{
}

Scheduler::~Scheduler() = default;

static QDateTime nextUpdate(Feed *feed)
{
    const QDateTime &updateStartTime = feed->updater()->updateStartTime();
    QDateTime lastUpdate{updateStartTime.isValid() ? updateStartTime : feed->lastUpdate()};
    return lastUpdate.addSecs(feed->updateInterval());
}

static bool needsUpdate(Feed *feed, const QDateTime &timestamp)
{
    return nextUpdate(feed) < timestamp;
}

void insertIntoSchedule(QList<Feed *> &schedule, Feed *feed)
{
    if (feed->updateMode() == Feed::DisableUpdateMode || feed->updateInterval() <= 0) {
        return;
    }
    const QDateTime &updateTime{nextUpdate(feed)};
    for (auto i = schedule.begin(); i != schedule.end(); ++i) {
        if (nextUpdate(*i) >= updateTime) {
            schedule.insert(i, feed);
            return;
        }
    }
    schedule.append(feed);
}

void Scheduler::schedule(Feed *feed, const QDateTime &timestamp)
{
    QObject::connect(feed, &Feed::statusChanged, this, [this, feed] {
        onFeedStatusChanged(feed);
    });
    QObject::connect(feed, &Feed::updateIntervalChanged, this, [this, feed] {
        reschedule(feed);
    });
    QObject::connect(feed, &QObject::destroyed, this, [this, feed] {
        d->schedule.removeAll(feed);
    });
    reschedule(feed, timestamp);
}

void Scheduler::unschedule(Feed *feed)
{
    d->schedule.removeOne(feed);
    QObject::disconnect(feed, nullptr, this, nullptr);
}

void Scheduler::start(int resolution)
{
    d->timer.setInterval(resolution);
    d->timer.callOnTimeout(this, &Scheduler::updateStale);
    d->timer.start();

    // also update immediately, in case anything was scheduled while we were stopped
    updateStale();
}

void Scheduler::stop()
{
    d->timer.stop();
}

bool Scheduler::isRunning()
{
    return d->timer.isActive();
}

static void updateMany(const QDateTime &timestamp, const QList<Feed::Updater *> &toUpdate)
{
    for (auto *entry : toUpdate) {
        entry->start(timestamp);
    }
}

void Scheduler::updateStale()
{
    // find all the stale feeds before we start updating them so that we don't modify the schedule while we're searching it...
    const auto &timestamp = QDateTime::currentDateTime();
    QList<Feed::Updater *> toUpdate{};
    const auto &schedule{d->schedule};
    for (Feed *entry : schedule) {
        if (!needsUpdate(entry, timestamp)) {
            break;
        }
        toUpdate << entry->updater();
    }
    updateMany(timestamp, toUpdate);
}

void Scheduler::clearErrors()
{
    QList<Feed *> errorFeeds;
    for (Feed *feed : qAsConst(d->schedule)) {
        if (feed->status() == Feed::Error) {
            errorFeeds << feed;
        }
    }
    QDateTime timestamp{QDateTime::currentDateTime()};
    for (Feed *feed : qAsConst(errorFeeds)) {
        feed->updater()->start(timestamp);
    }
}

void Scheduler::reschedule(Feed *feed, const QDateTime &timestamp)
{
    d->schedule.removeOne(feed);
    if (feed->status() == LoadStatus::Updating) {
        return;
    }
    if (isRunning() && needsUpdate(feed, timestamp)) {
        feed->updater()->start(timestamp);
    } else {
        insertIntoSchedule(d->schedule, feed);
    }
}

void Scheduler::onFeedStatusChanged(Feed *sender)
{
    if (sender->status() == LoadStatus::Updating) {
        d->schedule.removeOne(sender);
    } else {
        insertIntoSchedule(d->schedule, sender);
    }
}

}
