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
    QDateTime lastUpdate;
    QDateTime updateStartTime;
    QString errorMsg;
    bool active { false };
};

Updater::Updater(Feed *feed, QObject *parent) :
    QObject(parent),
    priv{ std::make_unique<PrivData>() }
{
    priv->feed = feed;
}

Updater::~Updater() = default;

void Updater::start(const QDateTime &timestamp)
{
    if (priv->feed->status() != LoadStatus::Updating) {
        priv->feed->setStatus(LoadStatus::Updating);
        run();
    }
    priv->updateStartTime = timestamp;
}

QString Updater::error()
{
    return priv->errorMsg;
}

Feed *Updater::feed()
{
    return priv->feed;
}

QDateTime Updater::nextUpdate()
{
    return priv->updateStartTime.addSecs(priv->updateInterval);
}

bool Updater::hasNextUpdate()
{
    return (priv->updateMode != ManualUpdateMode
            && priv->updateInterval > 0);
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
    return priv->lastUpdate;
}

void Updater::setLastUpdate(const QDateTime &lastUpdate)
{
    if (priv->lastUpdate != lastUpdate) {
        priv->lastUpdate = priv->updateStartTime = lastUpdate;
        emit lastUpdateChanged();
    }
}

Updater::UpdateMode Updater::updateMode()
{
    return priv->updateMode;
}

void Updater::setUpdateMode(Updater::UpdateMode updateMode)
{
    if (updateMode != priv->updateMode) {
        priv->updateMode = updateMode;
        emit updateModeChanged();
    }
}

qint64 Updater::updateInterval()
{
    return priv->updateInterval;
}

void Updater::setUpdateInterval(qint64 updateInterval)
{
    if (updateInterval != priv->updateInterval) {
        priv->updateInterval = updateInterval;
        emit updateIntervalChanged();
    }
}

void Updater::setDefaultUpdateInterval(qint64 updateInterval)
{
    if (priv->updateMode == DefaultUpdateMode) {
        setUpdateInterval(updateInterval);
    }
}

void Updater::updateParams(Updater *other)
{
    setUpdateInterval(other->updateInterval());
    setUpdateMode(other->updateMode());
}

void Updater::finish()
{
    setLastUpdate(priv->updateStartTime);
    priv->feed->setStatus(LoadStatus::Idle);
}

void Updater::setError(const QString &errorMsg)
{
    qDebug() << errorMsg;
    priv->errorMsg = errorMsg;
    priv->feed->setStatus(LoadStatus::Error);
}

}
