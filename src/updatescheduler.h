#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <Syndication/Feed>
#include <time.h>

#include "loadstatus.h"
class FeedUpdater;

class UpdateScheduler : public QObject
{
    Q_OBJECT
public:
    explicit UpdateScheduler(QObject *parent = nullptr);
    void schedule(qint64 feedId, QUrl url, time_t updateInterval, time_t lastUpdate);
    void unschedule(qint64 feedId);
    void start(int resolution=60000);
    void stop();
    void updateStale();
    void updateAll();

signals:
    void feedLoaded(FeedUpdater *updater, Syndication::FeedPtr feed);
    void feedStatusChanged(FeedUpdater *updater, LoadStatus status);

private:
    QList<FeedUpdater *> m_schedule;
    QTimer m_timer;
};

#endif // UPDATESCHEDULER_H
