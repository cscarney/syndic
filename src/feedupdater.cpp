#include "feedupdater.h"
#include <QTimer>
#include <QDebug>

struct FeedUpdater::PrivData {
    qint64 feedId;
    time_t updateInterval;
    time_t lastUpdate;
    LoadStatus status = LoadStatus::Idle;
    QString errorMsg;
};


FeedUpdater::FeedUpdater(qint64 feedId, time_t updateInterval, time_t lastUpdate, QObject *parent) :
    priv(std::make_unique<PrivData>())
{
    priv->feedId = feedId;
    priv->updateInterval = updateInterval;
    priv->lastUpdate = lastUpdate;
}

FeedUpdater::~FeedUpdater() = default;

float FeedUpdater::progress()
{
    return (status() == LoadStatus::Updating) ? 0.1f : 0;
}

void FeedUpdater::start()
{
    time_t t;
    time(&t);
    start(t);
}

void FeedUpdater::start(time_t timestamp)
{
    if (status() != LoadStatus::Updating) {
        setStatus(LoadStatus::Updating);
        run();
    }
    priv->lastUpdate = timestamp;
}

LoadStatus FeedUpdater::status()
{
    return priv->status;
}

QString FeedUpdater::error()
{
    return priv->errorMsg;
}

qint64 FeedUpdater::feedId()
{
    return priv->feedId;
}

time_t FeedUpdater::nextUpdate()
{
    return priv->lastUpdate + priv->updateInterval;
}

bool FeedUpdater::needsUpdate(time_t timestamp)
{
    return (nextUpdate() <= timestamp);
}

bool FeedUpdater::updateIfNecessary(time_t timestamp)
{
    if (needsUpdate(timestamp)) {
        start(timestamp);
        return true;
    } else {
        return false;
    }
}

void FeedUpdater::finish(Syndication::FeedPtr content)
{
    emit feedLoaded(this, content);
    setStatus(LoadStatus::Idle);
}

void FeedUpdater::setStatus(LoadStatus status)
{
    priv->status = status;
    emit statusChanged(this, status);
}

void FeedUpdater::setError(const QString &errorMsg)
{
    priv->errorMsg = errorMsg;
    setStatus(LoadStatus::Error);
}
