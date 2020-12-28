#include "updatescheduler.h"

#include <QDebug>

#include "xmlfeedupdater.h"
#include "feed.h"

namespace FeedCore {

UpdateScheduler::UpdateScheduler(QObject *parent) : QObject(parent)
{
}

inline void insertIntoSchedule(QList<Feed *> &schedule, Feed *feed)
{
    FeedUpdater *updater { feed->updater() };
    time_t nextUpdate { updater->nextUpdate() };
    for (auto i=schedule.begin(); i!=schedule.end(); ++i) {
        if ((*i)->updater()->nextUpdate() > nextUpdate) {
            schedule.insert(i, feed);
            return;
        }
    }
    schedule.append(feed);
}

void UpdateScheduler::schedule(const FeedRef &feedRef, time_t timestamp)
{
    unschedule(feedRef);     // there can only be one
    m_feeds.insert(feedRef);
    Feed *feed { feedRef.get() };
    QObject::connect(feed, &Feed::statusChanged, this,
                     [this, feed]{ onFeedStatusChanged(feed); });
    insertIntoSchedule(m_schedule, feed);
    feed->updater()->updateIfNecessary(timestamp);
}

void UpdateScheduler::schedule(const FeedRef &feed)
{
    time_t timestamp;
    time(&timestamp);
    schedule(feed, timestamp);
}

void UpdateScheduler::schedule(FeedQuery *q)
{
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q] {
        time_t timestamp;
        time(&timestamp);
        for (const auto &i : q->result()) {
            schedule(i, timestamp);
        }
    });
}

void UpdateScheduler::unschedule(const FeedRef &feed)
{
    bool isScheduled { m_feeds.contains(feed) };
    if (!isScheduled) {
        return;
    }
    m_active.remove(feed.get());
    m_schedule.removeOne(feed.get());
    m_feeds.remove(feed);
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

static void updateMany(time_t timestamp, const QList<FeedUpdater *> &toUpdate)
{
    for (auto *entry : toUpdate)
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
    for (Feed *entry : schedule) {
        FeedUpdater *updater = entry->updater();
        if (!entry->updater()->needsUpdate(timestamp)) {
            break;
        }
         toUpdate << updater;
    }
    updateMany(timestamp, toUpdate);
}

void UpdateScheduler::updateAll()
{
    time_t timestamp;
    time(&timestamp);
    m_schedule.clear();
    const auto &feeds = m_feeds;
    for (const FeedRef &entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

bool UpdateScheduler::updatesInProgress()
{
    return !m_active.isEmpty();
}

void UpdateScheduler::onFeedStatusChanged(Feed *sender)
{
    if (sender->status() == LoadStatus::Updating) {
        m_active.insert(sender);
        m_schedule.removeOne(sender);
    } else {
        m_active.remove(sender);
        insertIntoSchedule(m_schedule, sender);
    }
}

}
