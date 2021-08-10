#include "scheduler.h"
#include <QSet>
#include <QNetworkConfigurationManager>
#include "updater.h"
#include "feed.h"

namespace FeedCore {

struct Scheduler::PrivData {
    QSet<Feed*> feeds;
    QList<Feed *> schedule;
    QTimer timer;
    qint64 updateInterval{ 0 };
    qint64 expireAge { 0 };
    QNetworkConfigurationManager ncm;
};

static void clearErrors(const QList<Feed*> &schedule)
{
    QList<Feed*> errorFeeds;
    for(Feed *feed : schedule) {
        if (feed->status() == Feed::Error) {
            errorFeeds << feed;
        }
    }
    for(Feed *feed : qAsConst(errorFeeds)) {
        feed->updater()->start();
    }
}

Scheduler::Scheduler(QObject *parent) :
    QObject(parent),
    d(std::make_unique<PrivData>())
{
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationAdded, this, &Scheduler::onNetworkStateChanged);
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationChanged, this, &Scheduler::onNetworkStateChanged);
}

Scheduler::~Scheduler()=default;

void insertIntoSchedule(QList<Feed *> &schedule, Feed *feed)
{
    Updater *updater { feed->updater() };
    if (!updater->hasNextUpdate()) {
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

void Scheduler::schedule(Feed *feed, const QDateTime &timestamp)
{
    d->feeds.insert(feed);
    Updater *updater { feed->updater() };
    updater->setDefaultUpdateInterval(d->updateInterval);
    updater->setExpireAge(d->expireAge);
    QObject::connect(feed, &Feed::statusChanged, this,
                     [this, feed]{ onFeedStatusChanged(feed); });
    QObject::connect(updater, &Updater::updateIntervalChanged, this,
                     [this, feed]{ reschedule(feed); });
    QObject::connect(updater, &Updater::updateModeChanged, this,
                     [this, feed]{ onUpdateModeChanged(feed); });
    QObject::connect(feed, &QObject::destroyed, this,
                     [this, feed]{ unschedule(feed); });
    reschedule(feed, timestamp);
}

void Scheduler::schedule(Future<Feed*> *q)
{
    QObject::connect(q, &BaseFuture::finished, this, [this, q] {
        const auto &timestamp = QDateTime::currentDateTime();
        for (const auto &i : q->result()) {
            schedule(i, timestamp);
        }
    });
}

void Scheduler::unschedule(Feed *feed)
{
    bool isScheduled { d->feeds.contains(feed) };
    if (!isScheduled) {
        return;
    }
    d->schedule.removeOne(feed);
    d->feeds.remove(feed);
}

void Scheduler::start(int resolution)
{
    d->timer.setInterval(resolution);
    d->timer.callOnTimeout(this, &Scheduler::updateStale);
    d->timer.start();

    // also update immediately, in case anything was scheduled while we were stopped
    QTimer::singleShot(0, this, &Scheduler::updateStale);
}

void Scheduler::stop()
{
    d->timer.stop();
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
    const auto &schedule { d->schedule };
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
    d->schedule.clear();
    const auto &feeds = d->feeds;
    for (Feed *const entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

void Scheduler::abortAll()
{
    const auto &feeds = d->feeds;
    for(Feed *const entry : feeds) {
        entry->updater()->abort();
    }
}

qint64 Scheduler::updateInterval()
{
    return d->updateInterval;
}

void Scheduler::setUpdateInterval(qint64 newval){
    d->updateInterval = newval;
    const auto &feeds = d->feeds;
    for (const auto &feedRef : feeds) {
        feedRef->updater()->setDefaultUpdateInterval(newval);
    }
}

qint64 Scheduler::expireAge()
{
    return d->expireAge;
}

void Scheduler::setExpireAge(qint64 newval)
{
    d->expireAge = newval;
    const auto &feeds = d->feeds;
    for (const auto &feedRef : feeds) {
        feedRef->updater()->setExpireAge(newval);
    }
}

void Scheduler::reschedule(Feed *feed, const QDateTime &timestamp)
{
    Updater *updater { feed->updater() };
    d->schedule.removeOne(feed);
    if (updater->needsUpdate(timestamp)) {
        updater->start();
    } else {
        insertIntoSchedule(d->schedule, feed);
    }
}

void Scheduler::onUpdateModeChanged(Feed *feed)
{
    Updater *updater { feed->updater() };
    switch (updater->updateMode()) {
    case Updater::DefaultUpdateMode:
        updater->setDefaultUpdateInterval(d->updateInterval);
        break;

    case Updater::CustomUpdateMode:
        reschedule(feed);
        break;

    case Updater::ManualUpdateMode:
        d->schedule.removeOne(feed);
        break;
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

void Scheduler::onNetworkStateChanged()
{
    if (d->ncm.isOnline()) {
        clearErrors(d->schedule);
    }
}

}
