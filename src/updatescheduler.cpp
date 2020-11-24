#include "updatescheduler.h"
#include "xmlfeedupdater.h"

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
    QObject::connect(updater, &FeedUpdater::statusChanged, this, &UpdateScheduler::feedStatusChanged);
    updater->updateIfNecessary(timestamp);
    insertIntoSchedule(m_schedule, updater);
}

inline FeedUpdater *findInSchedule(QList<FeedUpdater *> &schedule, qint64 feedId, int &outIdx)
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

void UpdateScheduler::unschedule(qint64 feedId)
{
    int idx;
    auto entry = findInSchedule(m_schedule, feedId, idx);
    if (!entry) return;
    m_schedule.removeAt(idx);
    delete entry;
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

