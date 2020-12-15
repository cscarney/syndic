#include "updatescheduler.h"

#include <QDebug>

#include "xmlfeedupdater.h"
#include "feed.h"

namespace FeedCore {

UpdateScheduler::UpdateScheduler(QObject *parent) : QObject(parent)
{
}

inline void insertIntoSchedule(QList<FeedUpdater *> &schedule, FeedUpdater *updater)
{
    int nextUpdate = updater->nextUpdate();
    for (int i=0; i<schedule.length(); i++) {
        if (schedule[i]->nextUpdate() > nextUpdate) {
            schedule.insert(i, updater);
            return;
        }
    }
    schedule.append(updater);
}

void UpdateScheduler::schedule(const FeedRef &feed, time_t updateInterval, time_t lastUpdate, time_t timestamp)
{
    unschedule(feed);     // there can only be one
    auto *updater = new XMLFeedUpdater(feed, updateInterval, lastUpdate, this);
    QObject::connect(updater, &FeedUpdater::feedLoaded, this, &UpdateScheduler::feedLoaded);
    m_updaters.insert(feed, updater);
    updater->updateIfNecessary(timestamp);
    insertIntoSchedule(m_schedule, updater);
}

void UpdateScheduler::schedule(const FeedRef &feed, time_t updateInterval, time_t lastUpdate)
{
    time_t timestamp;
    time(&timestamp);
    schedule(feed, updateInterval, lastUpdate, timestamp);
}

void UpdateScheduler::schedule(FeedQuery *q)
{
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q] {
        time_t timestamp;
        time(&timestamp);
        for (const auto &i : q->result()) {
            schedule(i, 3600, 0);
        }
    });
}

void UpdateScheduler::unschedule(const FeedRef &feed)
{
    auto *updater = m_updaters.take(feed);
    if (updater == nullptr) {
        return;
    }
    m_schedule.removeOne(updater);
    delete updater;
}

void UpdateScheduler::start(int resolution)
{
    m_timer.setInterval(resolution);
    m_timer.callOnTimeout(this, &UpdateScheduler::updateStale);
    m_timer.start();

    // also update immediately, in case anything was scheduled while we were stopped
    QTimer::singleShot(0, this, &UpdateScheduler::updateStale);
}

void UpdateScheduler::stop()
{
    m_timer.stop();
}

void UpdateScheduler::update(const FeedRef &feed)
{
    auto *entry = m_updaters[feed];
    if (entry == nullptr) {
        qDebug() << "called UpdateScheduler::update with a feed id that has no updater";
        return;
    }
    m_schedule.removeOne(entry);
    entry->start();
    insertIntoSchedule(m_schedule, entry);
}

void UpdateScheduler::updateStale()
{
    time_t timestamp;
    time(&timestamp);
    if (m_schedule.isEmpty()) {
        return;
    }
    while (auto *entry=m_schedule.constFirst()) {
        if (!entry->updateIfNecessary(timestamp)) {
            return;
        }
        m_schedule.removeFirst();
        insertIntoSchedule(m_schedule, entry);
    }
}

void UpdateScheduler::updateAll()
{
    time_t timestamp;
    time(&timestamp);
    m_schedule.clear();
    const auto &updaters = m_updaters;
    for (auto *entry : updaters) {
        entry->start(timestamp);
        insertIntoSchedule(m_schedule, entry);
    }
}

LoadStatus UpdateScheduler::getStatus(const FeedRef &feed)
{
    auto *entry = m_updaters[feed];
    if (entry == nullptr) {
        return LoadStatus::Idle;
    }
    return entry->status();
}

bool UpdateScheduler::updatesInProgress()
{
    return !m_active.isEmpty();
}

}
