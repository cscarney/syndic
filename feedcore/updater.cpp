#include "updater.h"
#include <QTimer>
#include "feed.h"

namespace FeedCore {

struct Updater::PrivData {
    Feed *feed;
    time_t updateInterval { 0 };
    time_t lastUpdate { 0 };
    QString errorMsg;
    bool active { false };
};


Updater::Updater(Feed *feed, time_t updateInterval, time_t lastUpdate, QObject *parent) :
    QObject(parent),
    priv{ std::make_unique<PrivData>() }
{
    priv->feed = feed;
    priv->updateInterval = updateInterval;
    priv->lastUpdate = lastUpdate;
}

Updater::~Updater() = default;

void Updater::start()
{
    time_t t;
    time(&t);
    start(t);
}

void Updater::start(time_t timestamp)
{
    if (priv->feed->status() != LoadStatus::Updating) {
        priv->feed->setStatus(LoadStatus::Updating);
        run();
    }
    priv->lastUpdate = timestamp;
}

QString Updater::error()
{
    return priv->errorMsg;
}

Feed *Updater::feed()
{
    return priv->feed;
}

time_t Updater::nextUpdate()
{
    return priv->lastUpdate + priv->updateInterval;
}

bool Updater::needsUpdate(time_t timestamp)
{
    return (nextUpdate() <= timestamp);
}

bool Updater::updateIfNecessary(time_t timestamp)
{
    if (needsUpdate(timestamp)) {
        start(timestamp);
        return true;
    }
    return false;
}

void Updater::finish()
{
    priv->feed->setStatus(LoadStatus::Idle);
}

void Updater::setError(const QString &errorMsg)
{
    priv->errorMsg = errorMsg;
    priv->feed->setStatus(LoadStatus::Error);
}

}
