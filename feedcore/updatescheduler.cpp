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
    time_t nextUpdate { updater->nextUpdate() };
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
    auto *const updater = new XMLFeedUpdater(feed, updateInterval, lastUpdate, this);
    m_updaters.insert(feed, updater);
    QObject::connect(updater, &FeedUpdater::activeChanged, this,
                     [this, updater]{ onUpdaterActiveChanged(updater); });
    insertIntoSchedule(m_schedule, updater);
    updater->updateIfNecessary(timestamp);
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
    FeedUpdater *const entry = m_updaters[feed];
    if (entry == nullptr) {
        qDebug() << "called UpdateScheduler::update with a feed id that has no updater";
        return;
    }
    m_schedule.removeOne(entry);
    entry->start();
    insertIntoSchedule(m_schedule, entry);
}

static void updateMany(time_t timestamp, const QList<FeedUpdater *> &toUpdate)
{
    for (auto *const entry : toUpdate)
    {
        entry->start(timestamp);
    }
}

void UpdateScheduler::updateStale()
{
    time_t timestamp;
    time(&timestamp);
    QList<FeedUpdater *> toUpdate{};
    const auto &schedule { m_schedule };
    for (auto *const entry : schedule) {
        if (!entry->needsUpdate(timestamp)) {
            break;
        }
         toUpdate << entry;
    }
    updateMany(timestamp, toUpdate);
}

void UpdateScheduler::updateAll()
{
    time_t timestamp;
    time(&timestamp);
    m_schedule.clear();
    const auto &updaters = m_updaters;
    for (auto *const entry : updaters) {
        entry->start(timestamp);
        insertIntoSchedule(m_schedule, entry);
    }
}

bool UpdateScheduler::updatesInProgress()
{
    return !m_active.isEmpty();
}

void UpdateScheduler::onUpdaterActiveChanged(FeedUpdater *sender)
{
    if (sender->active()) {
        m_active.insert(sender->feed());
        m_schedule.removeOne(sender);
    } else {
        m_active.remove(sender->feed());
        insertIntoSchedule(m_schedule, sender);
    }
}

}
