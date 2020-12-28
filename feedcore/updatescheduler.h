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

    void schedule(const FeedRef &feedRef, time_t timestamp);
    void schedule(const FeedRef &feedRef);
    void schedule(FeedQuery *q);
    void unschedule(const FeedRef &feedRef);
    void start(int resolution=60000);
    void stop();
    void updateStale();
    void updateAll();
    bool updatesInProgress();

private:
    QSet<FeedRef> m_feeds;
    QList<Feed *> m_schedule;
    QSet<Feed *> m_active;
    QTimer m_timer;

    void onFeedStatusChanged(Feed *sender);
};

}

#endif // UPDATESCHEDULER_H
