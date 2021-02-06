#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QSet>
#include <QDateTime>
#include <Syndication/Feed>
#include "enums.h"
#include "future.h"

namespace FeedCore {
class Feed;
class Updater;

class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(QObject *parent = nullptr);
    void schedule(Feed *feed, const QDateTime &timestamp=QDateTime::currentDateTime());
    void schedule(Feed *feedRef);
    void schedule(Future<Feed*> *q);
    void unschedule(Feed *feedRef);
    void start(int resolution=60000);
    void stop();
    void updateStale();
    void updateAll();
    void abortAll();
    qint64 updateInterval();
    void setUpdateInterval(qint64 newval);
private:
    QSet<Feed*> m_feeds;
    QList<Feed *> m_schedule;
    QTimer m_timer;
    qint64 m_updateInterval{ 0 };
    void reschedule(Feed *feed, const QDateTime &timestamp=QDateTime::currentDateTime());
    void onUpdateModeChanged(Feed *feed);
    void onFeedStatusChanged(Feed *sender);
};
}
#endif // SCHEDULER_H
