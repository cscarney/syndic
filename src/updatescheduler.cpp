#include "updatescheduler.h"
#include "xmlfeedupdater.h"

#include <QDebug>

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

void UpdateScheduler::schedule(qint64 feedId, QUrl url, time_t updateInterval, time_t lastUpdate)
{
    unschedule(feedId);     // there can only be one
    time_t timestamp;
    time(&timestamp);
    auto *updater = new XMLFeedUpdater(feedId, url, updateInterval, lastUpdate, this);
    QObject::connect(updater, &FeedUpdater::feedLoaded, this, &UpdateScheduler::feedLoaded);
    QObject::connect(updater, &FeedUpdater::statusChanged, this, &UpdateScheduler::slotFeedStatusChanged);
    m_updaters.insert(feedId, updater);
    updater->updateIfNecessary(timestamp);
    insertIntoSchedule(m_schedule, updater);
}

/*
inline FeedUpdater *findInSchedule(QList<FeedUpdater *> &schedule, FeedUpdater *needle, int &outIdx)
{
    auto len = schedule.length();
    for (outIdx=0; outIdx<len; outIdx++) {
        auto *entry = schedule[outIdx];
        if (schedule[outIdx]->feedId() == feedId) {
            return entry;
        }
    }
    return nullptr;
}
*/

void UpdateScheduler::unschedule(qint64 feedId)
{
    auto *updater = m_updaters.take(feedId);
    if (!updater) return;
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

void UpdateScheduler::update(qint64 feedId)
{
    auto *entry = m_updaters[feedId];
    if (entry) {
        m_schedule.removeOne(entry);
        entry->start();
        insertIntoSchedule(m_schedule, entry);
    } else {
        qDebug() << "called UpdateScheduler::update with a feed id that has no updater";
    }
}

void UpdateScheduler::updateStale()
{
    time_t timestamp;
    time(&timestamp);
    if (m_schedule.isEmpty()) return;
    while (auto entry=m_schedule.constFirst()) {
        if (!entry->updateIfNecessary(timestamp))
            return;
        m_schedule.removeFirst();
        insertIntoSchedule(m_schedule, entry);
    }
}

void UpdateScheduler::updateAll()
{
    time_t timestamp;
    time(&timestamp);
    m_schedule.clear();
    for (auto *entry : m_updaters) {
        entry->start(timestamp);
        insertIntoSchedule(m_schedule, entry);
    }
}

LoadStatus UpdateScheduler::getStatus(qint64 feedId)
{
    auto *entry = m_updaters[feedId];
    if (!entry) return LoadStatus::Idle;
    return entry->status();
}

bool UpdateScheduler::updatesInProgress()
{
    return !m_active.isEmpty();
}

void UpdateScheduler::slotFeedStatusChanged(FeedUpdater *updater, LoadStatus status)
{
    if (status == LoadStatus::Updating) {
        m_active << updater->feedId();
    } else {
        m_active.remove(updater->feedId());
    }
    emit feedStatusChanged(updater, status);
}

