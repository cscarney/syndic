#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QSet>
#include <Syndication/Feed>
#include <time.h>
#include "enums.h"
#include "future.h"
#include "feedref.h"

namespace FeedCore {
class Updater;

class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(QObject *parent = nullptr);
    ~Scheduler();
    void schedule(const FeedRef &feedRef, time_t timestamp);
    void schedule(const FeedRef &feedRef);
    void schedule(Future<FeedRef> *q);
    void unschedule(const FeedRef &feedRef);
    void start(int resolution=60000);
    void stop();
    void updateStale();
    void updateAll();
    void abortAll();
private:
    QSet<FeedRef> m_feeds;
    QList<Feed *> m_schedule;
    QTimer m_timer;
    void onFeedStatusChanged(Feed *sender);
};
}
#endif // SCHEDULER_H
