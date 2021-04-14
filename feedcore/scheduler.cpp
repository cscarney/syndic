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
        if (feed->status() == Enums::Error) {
            errorFeeds << feed;
        }
    }
    for(Feed *feed : errorFeeds) {
        feed->updater()->start();
    }
}

Scheduler::Scheduler(QObject *parent) :
    QObject(parent),
    priv(std::make_unique<PrivData>())
{
    QObject::connect(&priv->ncm, &QNetworkConfigurationManager::onlineStateChanged, this, &Scheduler::onNetworkStateChanged);
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
    priv->feeds.insert(feed);
    Updater *updater { feed->updater() };
    updater->setDefaultUpdateInterval(priv->updateInterval);
    updater->setExpireAge(priv->expireAge);
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
    bool isScheduled { priv->feeds.contains(feed) };
    if (!isScheduled) {
        return;
    }
    priv->schedule.removeOne(feed);
    priv->feeds.remove(feed);
}

void Scheduler::start(int resolution)
{
    priv->timer.setInterval(resolution);
    priv->timer.callOnTimeout(this, &Scheduler::updateStale);
    priv->timer.start();

    // also update immediately, in case anything was scheduled while we were stopped
    QTimer::singleShot(0, this, &Scheduler::updateStale);
}

void Scheduler::stop()
{
    priv->timer.stop();
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
    const auto &schedule { priv->schedule };
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
    priv->schedule.clear();
    const auto &feeds = priv->feeds;
    for (Feed *const entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

void Scheduler::abortAll()
{
    const auto &feeds = priv->feeds;
    for(Feed *const entry : feeds) {
        entry->updater()->abort();
    }
}

qint64 Scheduler::updateInterval()
{
    return priv->updateInterval;
}

void Scheduler::setUpdateInterval(qint64 newval){
    priv->updateInterval = newval;
    const auto &feeds = priv->feeds;
    for (const auto &feedRef : feeds) {
        feedRef->updater()->setDefaultUpdateInterval(newval);
    }
}

qint64 Scheduler::expireAge()
{
    return priv->expireAge;
}

void Scheduler::setExpireAge(qint64 newval)
{
    priv->expireAge = newval;
    const auto &feeds = priv->feeds;
    for (const auto &feedRef : feeds) {
        feedRef->updater()->setExpireAge(newval);
    }
}

void Scheduler::reschedule(Feed *feed, const QDateTime &timestamp)
{
    Updater *updater { feed->updater() };
    priv->schedule.removeOne(feed);
    if (updater->needsUpdate(timestamp)) {
        updater->start();
    } else {
        insertIntoSchedule(priv->schedule, feed);
    }
}

void Scheduler::onUpdateModeChanged(Feed *feed)
{
    Updater *updater { feed->updater() };
    switch (updater->updateMode()) {
    case Updater::DefaultUpdateMode:
        updater->setDefaultUpdateInterval(priv->updateInterval);
        break;

    case Updater::CustomUpdateMode:
        reschedule(feed);
        break;

    case Updater::ManualUpdateMode:
        priv->schedule.removeOne(feed);
        break;
    }
}

void Scheduler::onFeedStatusChanged(Feed *sender)
{
    if (sender->status() == LoadStatus::Updating) {
        priv->schedule.removeOne(sender);
    } else {
        insertIntoSchedule(priv->schedule, sender);
    }
}

void Scheduler::onNetworkStateChanged()
{
    if (priv->ncm.isOnline()) {
        clearErrors(priv->schedule);
    }
}

}
