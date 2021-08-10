#include "updater.h"
#include <QTimer>
#include <QNetworkConfigurationManager>
#include <QDebug>
#include "feed.h"

namespace FeedCore {

struct Updater::PrivData {
    Feed *feed;
    UpdateMode updateMode { DefaultUpdateMode };
    time_t updateInterval { 0 };
    qint64 expireAge { 0 };
    QDateTime lastUpdate;
    QDateTime updateStartTime;
    QString errorMsg;
    bool active { false };
};

Updater::Updater(Feed *feed, QObject *parent) :
    QObject(parent),
    d{ std::make_unique<PrivData>() }
{
    d->feed = feed;
}

Updater::~Updater() = default;

void Updater::start(const QDateTime &timestamp)
{
    if (d->feed->status() != LoadStatus::Updating) {
        d->feed->setStatus(LoadStatus::Updating);
        if (d->expireAge > 0) {
            emit expire(timestamp.addSecs(-d->expireAge));
        }
        run();
    }
    d->updateStartTime = timestamp;
}

QString Updater::error()
{
    return d->errorMsg;
}

Feed *Updater::feed()
{
    return d->feed;
}

QDateTime Updater::nextUpdate()
{
    return d->updateStartTime.addSecs(d->updateInterval);
}

bool Updater::hasNextUpdate()
{
    return (d->updateMode != ManualUpdateMode
            && d->updateInterval > 0);
}

bool Updater::needsUpdate(const QDateTime &timestamp)
{
    return hasNextUpdate() && nextUpdate() <= timestamp;
}

bool Updater::updateIfNecessary(const QDateTime &timestamp)
{
    if (needsUpdate(timestamp)) {
        start(timestamp);
        return true;
    }
    return false;
}

const QDateTime &Updater::lastUpdate()
{
    return d->lastUpdate;
}

void Updater::setLastUpdate(const QDateTime &lastUpdate)
{
    if (d->lastUpdate != lastUpdate) {
        d->lastUpdate = d->updateStartTime = lastUpdate;
        emit lastUpdateChanged();
    }
}

Updater::UpdateMode Updater::updateMode()
{
    return d->updateMode;
}

void Updater::setUpdateMode(Updater::UpdateMode updateMode)
{
    if (updateMode != d->updateMode) {
        d->updateMode = updateMode;
        emit updateModeChanged();
    }
}

qint64 Updater::updateInterval()
{
    return d->updateInterval;
}

void Updater::setUpdateInterval(qint64 updateInterval)
{
    if (updateInterval != d->updateInterval) {
        d->updateInterval = updateInterval;
        emit updateIntervalChanged();
    }
}

void Updater::setDefaultUpdateInterval(qint64 updateInterval)
{
    if (d->updateMode == DefaultUpdateMode) {
        setUpdateInterval(updateInterval);
    }
}

void Updater::setExpireAge(qint64 expireAge)
{
    d->expireAge = expireAge;
}

void Updater::updateParams(Updater *other)
{
    setUpdateInterval(other->updateInterval());
    setUpdateMode(other->updateMode());
}

void Updater::finish()
{
    setLastUpdate(d->updateStartTime);
    d->feed->setStatus(LoadStatus::Idle);
}

void Updater::setError(const QString &errorMsg)
{
    d->errorMsg = errorMsg;
    d->feed->setStatus(LoadStatus::Error);
}

}
