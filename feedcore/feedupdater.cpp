#include "feedupdater.h"

#include <QTimer>

#include "feed.h"

namespace FeedCore {

struct FeedUpdater::PrivData {
    Feed *feed;
    time_t updateInterval = 0;
    time_t lastUpdate = 0;
    QString errorMsg;
    bool active = false;
};


FeedUpdater::FeedUpdater(Feed *feed, time_t updateInterval, time_t lastUpdate, QObject *parent) :
    QObject(parent),
    priv(std::make_unique<PrivData>())
{
    priv->feed = feed;
    priv->updateInterval = updateInterval;
    priv->lastUpdate = lastUpdate;
}

FeedUpdater::~FeedUpdater() = default;

void FeedUpdater::start()
{
    time_t t;
    time(&t);
    start(t);
}

void FeedUpdater::start(time_t timestamp)
{
    if (priv->feed->status() != LoadStatus::Updating) {
        priv->feed->setStatus(LoadStatus::Updating);
        run();
    }
    priv->lastUpdate = timestamp;
}

QString FeedUpdater::error()
{
    return priv->errorMsg;
}

Feed *FeedUpdater::feed()
{
    return priv->feed;
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
    }
    return false;
}

void FeedUpdater::finish()
{
    priv->feed->setStatus(LoadStatus::Idle);
}

void FeedUpdater::setError(const QString &errorMsg)
{
    priv->errorMsg = errorMsg;
    priv->feed->setStatus(LoadStatus::Error);
}

}
