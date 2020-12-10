#ifndef FEEDUPDATER_H
#define FEEDUPDATER_H

#include <Syndication/Feed>
#include <QObject>
#include <time.h>
#include <memory>

#include "enums.h"
#include "feed.h"

namespace FeedCore {

class FeedUpdater : public QObject
{
    Q_OBJECT
public:
    explicit FeedUpdater(const FeedRef &feed, time_t updateInterval, time_t lastUpdate, QObject *parent = nullptr);
    ~FeedUpdater();

    virtual float progress();
    virtual void run() = 0;
    virtual void cancel() {}

    void start();
    void start(time_t timestamp);
    LoadStatus status();
    QString error();
    FeedRef feed();
    time_t nextUpdate();
    bool needsUpdate(time_t timestamp);
    bool updateIfNecessary(time_t timestamp);

signals:
    void feedLoaded(FeedCore::FeedUpdater *sender, const Syndication::FeedPtr &content);
    void statusChanged(FeedCore::FeedUpdater *sender, FeedCore::LoadStatus status);

protected:
    void finish(const Syndication::FeedPtr &content);
    void setStatus(LoadStatus status);
    void setError(const QString &errorMsg);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};

}

#endif // FEEDUPDATER_H
