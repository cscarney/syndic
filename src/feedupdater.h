#ifndef FEEDUPDATER_H
#define FEEDUPDATER_H

#include <Syndication/Feed>
#include <QObject>
#include <time.h>
#include <memory>

#include "loadstatus.h"

class FeedUpdater : public QObject
{
    Q_OBJECT
public:
    explicit FeedUpdater(qint64 feedId, time_t updateInterval, time_t lastUpdate, QObject *parent = nullptr);
    ~FeedUpdater();

    virtual float progress();
    virtual void run() = 0;
    virtual void cancel() {}

    void start();
    void start(time_t timestamp);
    LoadStatus status();
    QString error();
    qint64 feedId();
    time_t nextUpdate();
    bool needsUpdate(time_t timestamp);
    bool updateIfNecessary(time_t timestamp);

signals:
    void feedLoaded(FeedUpdater *sender, Syndication::FeedPtr content);
    void statusChanged(FeedUpdater *sender, LoadStatus status);

protected:
    void finish(Syndication::FeedPtr content);
    void setStatus(LoadStatus status);
    void setError(const QString &errorMsg);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};

#endif // FEEDUPDATER_H
