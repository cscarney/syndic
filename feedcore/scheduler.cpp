/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "scheduler.h"
#include "subscription.h"
#include <QSet>
#include <QTimer>

namespace FeedCore
{
struct Scheduler::PrivData {
    QList<Subscription *> schedule;
    QTimer timer;
};

Scheduler::Scheduler(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<PrivData>())
{
}

Scheduler::~Scheduler() = default;

static QDateTime nextUpdate(Subscription *feed)
{
    const QDateTime &updateStartTime = feed->updater()->updateStartTime();
    QDateTime lastUpdate{updateStartTime.isValid() ? updateStartTime : feed->lastUpdate()};
    return lastUpdate.addSecs(feed->updateInterval());
}

static bool needsUpdate(Subscription *feed, const QDateTime &timestamp)
{
    return nextUpdate(feed) < timestamp;
}

void insertIntoSchedule(QList<Subscription *> &schedule, Subscription *feed)
{
    if (feed->updateMode() == Subscription::DisableUpdateMode || feed->updateInterval() <= 0) {
        return;
    }
    const QDateTime &updateTime{nextUpdate(feed)};
    for (auto i = schedule.cbegin(); i != schedule.cend(); ++i) {
        if (nextUpdate(*i) >= updateTime) {
            schedule.insert(i, feed);
            return;
        }
    }
    schedule.append(feed);
}

void Scheduler::schedule(Subscription *feed, const QDateTime &timestamp)
{
    QObject::connect(feed, &Feed::statusChanged, this, [this, feed] {
        onFeedStatusChanged(feed);
    });
    QObject::connect(feed, &Subscription::updateIntervalChanged, this, [this, feed] {
        reschedule(feed);
    });
    QObject::connect(feed, &QObject::destroyed, this, [this, feed] {
        d->schedule.removeAll(feed);
    });
    reschedule(feed, timestamp);
}

void Scheduler::unschedule(Subscription *feed)
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

static void updateMany(const QDateTime &timestamp, const QList<Subscription::Updater *> &toUpdate)
{
    for (auto *entry : toUpdate) {
        entry->start(timestamp);
    }
}

void Scheduler::updateStale()
{
    // find all the stale feeds before we start updating them so that we don't modify the schedule while we're searching it...
    const auto &timestamp = QDateTime::currentDateTime();
    QList<Subscription::Updater *> toUpdate{};
    const auto &schedule{d->schedule};
    for (Subscription *entry : schedule) {
        if (!needsUpdate(entry, timestamp)) {
            break;
        }
        toUpdate << entry->updater();
    }
    updateMany(timestamp, toUpdate);
}

void Scheduler::clearErrors()
{
    QList<Subscription *> errorFeeds;
    for (Subscription *feed : std::as_const(d->schedule)) {
        if (feed->status() == Feed::Error) {
            errorFeeds << feed;
        }
    }
    QDateTime timestamp{QDateTime::currentDateTime()};
    for (Subscription *feed : std::as_const(errorFeeds)) {
        feed->updater()->start(timestamp);
    }
}

void Scheduler::reschedule(Subscription *feed, const QDateTime &timestamp)
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

void Scheduler::onFeedStatusChanged(Subscription *sender)
{
    if (sender->status() == LoadStatus::Updating) {
        d->schedule.removeOne(sender);
    } else {
        insertIntoSchedule(d->schedule, sender);
    }
}

}
