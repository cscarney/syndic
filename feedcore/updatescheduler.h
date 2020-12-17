#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QSet>
#include <Syndication/Feed>
#include <time.h>

#include "enums.h"
#include "feedstorageoperation.h"

namespace FeedCore {

class FeedUpdater;

class UpdateScheduler : public QObject
{
    Q_OBJECT
public:
    explicit UpdateScheduler(QObject *parent = nullptr);

    void add(qint64 feedId, QUrl url);
    void schedule(const FeedRef &feed, time_t updateInterval, time_t lastUpdate, time_t timestamp);
    void schedule(const FeedRef &feed, time_t updateInterval, time_t lastUpdate);
    void schedule(FeedQuery *q);
    void unschedule(const FeedRef &feed);
    void start(int resolution=60000);
    void stop();
    void update(const FeedRef &feed);
    void updateStale();
    void updateAll();
    bool updatesInProgress();

private:
    QList<FeedUpdater *> m_schedule;
    QTimer m_timer;
    QSet<FeedRef>m_active;
    QHash<FeedRef, FeedUpdater *> m_updaters;

    void onUpdaterActiveChanged(FeedUpdater *sender);
};

}

#endif // UPDATESCHEDULER_H
