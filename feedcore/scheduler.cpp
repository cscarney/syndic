#include "scheduler.h"
#include <QDebug>
#include "updater.h"
#include "feed.h"
#include "feedref.h"

namespace FeedCore {

Scheduler::Scheduler(QObject *parent) : QObject(parent)
{
}

void insertIntoSchedule(QList<Feed *> &schedule, Feed *feed)
{
    Updater *updater { feed->updater() };
    if (updater->updateMode() == Updater::MaunualUpdateMode) {
        return;
    }
    const QDateTime &nextUpdate { updater->nextUpdate() };
    for (auto i=schedule.begin(); i!=schedule.end(); ++i) {
        if ((*i)->updater()->nextUpdate() > nextUpdate) {
            schedule.insert(i, feed);
            return;
        }
    }
    schedule.append(feed);
}

static constexpr const qint64 defaultUpdateInterval = 3600;
void Scheduler::schedule(const FeedRef &feedRef, const QDateTime &timestamp)
{
    m_feeds.insert(feedRef);
    Feed *feed { feedRef.get() };
    Updater *updater { feed->updater() };
    updater->setDefaultUpdateInterval(defaultUpdateInterval);
    QObject::connect(feed, &Feed::statusChanged, this,
                     [this, feed]{ onFeedStatusChanged(feed); });
    QObject::connect(updater, &Updater::updateIntervalChanged, this,
                     [this, feed]{ reschedule(feed); });
    QObject::connect(updater, &Updater::updateModeChanged, this,
                     [this, feed]{ onUpdateModeChanged(feed); });
    reschedule(feed, timestamp);
}

void Scheduler::schedule(Future<FeedRef> *q)
{
    QObject::connect(q, &BaseFuture::finished, this, [this, q] {
        const auto &timestamp = QDateTime::currentDateTime();
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

static void updateMany(const QDateTime &timestamp, const QList<Updater *> &toUpdate)
{
    for (auto *entry : toUpdate)
    {
        entry->start(timestamp);
    }
}

void Scheduler::updateStale()
{
    const auto &timestamp = QDateTime::currentDateTime();
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
    const auto &timestamp = QDateTime::currentDateTime();
    m_schedule.clear();
    const auto &feeds = m_feeds;
    for (const FeedRef &entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

void Scheduler::abortAll()
{
    const auto &feeds = m_feeds;
    for(const FeedRef &entry : feeds) {
        entry->updater()->abort();
    }
}

void Scheduler::reschedule(Feed *feed, const QDateTime &timestamp)
{
    Updater *updater { feed->updater() };
    m_schedule.removeOne(feed);
    insertIntoSchedule(m_schedule, feed);
    updater->updateIfNecessary(timestamp);
}

void Scheduler::onUpdateModeChanged(Feed *feed)
{
    Updater *updater { feed->updater() };
    switch (updater->updateMode()) {
    case Updater::DefaultUpdateMode:
        updater->setDefaultUpdateInterval(defaultUpdateInterval);
        break;

    case Updater::CustomUpdateMode:
        reschedule(feed);
        break;

    case Updater::MaunualUpdateMode:
        m_schedule.removeOne(feed);
        break;
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
