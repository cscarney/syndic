#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <QObject>
#include <QDateTime>
#include <memory>
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
    ~Scheduler();
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
    qint64 expireAge();
    void setExpireAge(qint64 newval);
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
    void reschedule(Feed *feed, const QDateTime &timestamp=QDateTime::currentDateTime());
    void onUpdateModeChanged(Feed *feed);
    void onFeedStatusChanged(Feed *sender);
    void onNetworkStateChanged();
};
}
#endif // SCHEDULER_H
