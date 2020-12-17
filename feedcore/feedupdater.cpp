#include "feedupdater.h"

#include <QTimer>

#include "feed.h"

namespace FeedCore {

struct FeedUpdater::PrivData {
    FeedRef feed;
    time_t updateInterval = 0;
    time_t lastUpdate = 0;
    QString errorMsg;
    bool active = false;
};


FeedUpdater::FeedUpdater(const FeedRef &feed, time_t updateInterval, time_t lastUpdate, QObject *parent) :
    priv(std::make_unique<PrivData>())
{
    priv->feed = feed;
    priv->updateInterval = updateInterval;
    priv->lastUpdate = lastUpdate;
}

FeedUpdater::~FeedUpdater() = default;

float FeedUpdater::progress()
{
    return (active()) ? 0.1f : 0;
}

void FeedUpdater::start()
{
    time_t t;
    time(&t);
    start(t);
}

void FeedUpdater::start(time_t timestamp)
{
    if (!active()) {
        setActive(true);
        priv->feed->setStatus(LoadStatus::Updating);
        run();
    }
    priv->lastUpdate = timestamp;
}

QString FeedUpdater::error()
{
    return priv->errorMsg;
}

FeedRef FeedUpdater::feed()
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

bool FeedUpdater::active()
{
    return priv->active;
}

void FeedUpdater::finish(const Syndication::FeedPtr &content)
{
    priv->feed->updateFromSource(content);
    priv->feed->setStatus(LoadStatus::Idle);
    setActive(false);
}

void FeedUpdater::setError(const QString &errorMsg)
{
    priv->errorMsg = errorMsg;
    priv->feed->setStatus(LoadStatus::Error);
    setActive(false);
}

void FeedUpdater::setActive(bool active)
{
    if (priv->active != active) {
        priv->active = active;
        emit activeChanged();
    }
}

}
