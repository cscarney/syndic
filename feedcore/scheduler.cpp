#include "scheduler.h"
#include <QDebug>
#include "updater.h"
#include "feed.h"
#include "feedref.h"

namespace FeedCore {

Scheduler::Scheduler(QObject *parent) : QObject(parent)
{
}

Scheduler::~Scheduler()=default;

void insertIntoSchedule(QList<Feed *> &schedule, Feed *feed)
{
    Updater *updater { feed->updater() };
    time_t nextUpdate { updater->nextUpdate() };
    for (auto i=schedule.begin(); i!=schedule.end(); ++i) {
        if ((*i)->updater()->nextUpdate() > nextUpdate) {
            schedule.insert(i, feed);
            return;
        }
    }
    schedule.append(feed);
}

void Scheduler::schedule(const FeedRef &feedRef, time_t timestamp)
{
    unschedule(feedRef);     // there can only be one
    m_feeds.insert(feedRef);
    Feed *feed { feedRef.get() };
    QObject::connect(feed, &Feed::statusChanged, this,
                     [this, feed]{ onFeedStatusChanged(feed); });
    insertIntoSchedule(m_schedule, feed);
    feed->updater()->updateIfNecessary(timestamp);
}

void Scheduler::schedule(const FeedRef &feed)
{
    time_t timestamp;
    time(&timestamp);
    schedule(feed, timestamp);
}

void Scheduler::schedule(Future<FeedRef> *q)
{
    QObject::connect(q, &BaseFuture::finished, this, [this, q] {
        time_t timestamp;
        time(&timestamp);
        for (const auto &i : q->result()) {
            schedule(i, timestamp);
        }
    });
}

void Scheduler::unschedule(const FeedRef &feed)
{
    bool isScheduled { m_feeds.contains(feed) };
    if (!isScheduled) {
        return;
    }
    m_schedule.removeOne(feed.get());
    m_feeds.remove(feed);
}

void Scheduler::start(int resolution)
{
    m_timer.setInterval(resolution);
    m_timer.callOnTimeout(this, &Scheduler::updateStale);
    m_timer.start();

    // also update immediately, in case anything was scheduled while we were stopped
    QTimer::singleShot(0, this, &Scheduler::updateStale);
}

void Scheduler::stop()
{
    m_timer.stop();
}

static void updateMany(time_t timestamp, const QList<Updater *> &toUpdate)
{
    for (auto *entry : toUpdate)
    {
        entry->start(timestamp);
    }
}

void Scheduler::updateStale()
{
    time_t timestamp;
    time(&timestamp);
    QList<Updater *> toUpdate{};
    const auto &schedule { m_schedule };
    for (Feed *entry : schedule) {
        Updater *updater = entry->updater();
        if (!entry->updater()->needsUpdate(timestamp)) {
            break;
        }
         toUpdate << updater;
    }
    updateMany(timestamp, toUpdate);
}

void Scheduler::updateAll()
{
    time_t timestamp;
    time(&timestamp);
    m_schedule.clear();
    const auto &feeds = m_feeds;
    for (const FeedRef &entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

void Scheduler::onFeedStatusChanged(Feed *sender)
{
    if (sender->status() == LoadStatus::Updating) {
        m_schedule.removeOne(sender);
    } else {
        insertIntoSchedule(m_schedule, sender);
    }
}

}
