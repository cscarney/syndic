#ifndef FEEDCORE_SCHEDULER_H
#define FEEDCORE_SCHEDULER_H
#include <QObject>
#include <QDateTime>
#include <memory>
#include "feed.h"
#include "future.h"

namespace FeedCore {
class Updater;

/**
 * Automatically update feeds when they become stale
 */
class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(QObject *parent = nullptr);
    ~Scheduler();

    /**
     * Add a feed to the update schedule.
     *
     * If the feed is already stale as of /timestamp/ it is updated immediately and /timestamp/ is
     * recorded as the update time.
     */
    void schedule(Feed *feed, const QDateTime &timestamp=QDateTime::currentDateTime());

    /**
     * Wait on a future, then add its results to the update schedule.
     */
    void schedule(Future<Feed*> *q);

    /**
     * Remove a feed from the update schedule.
     */
    void unschedule(Feed *feedRef);

    /**
     * Start the update timer
     *
     * Once this method is called, stale feeds will be checked for every /resolution/ msecs
     * until stop() is called.
     */
    void start(int resolution=60000);

    /**
     * Stop the update timer
     */
    void stop();

    /**
     * Perform a one-time update of all stale feeds.
     */
    void updateStale();

    /**
     * Trigger an update on all feeds that are registered with the Scheduler
     */
    void updateAll();

    /**
     * Attempt to abort any pending updates
     */
    void abortAll();

    /**
     * The default update interval in seconds (for feeds using Updater::DefaultUpdateMode)
     */
    qint64 updateInterval();

    /**
     * Set the default update interval in seconds
     */
    void setUpdateInterval(qint64 newval);

    /**
     * The expire age for stale articles.
     *
     * This value is passed through to any scheduled feeds.
     */
    qint64 expireAge();

    /**
     * Set the expire age for stale articles.
     *
     * This value is passed through to any scheduled feeds.
     */
    void setExpireAge(qint64 newval);
private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    void reschedule(Feed *feed, const QDateTime &timestamp=QDateTime::currentDateTime());
    void onUpdateModeChanged(Feed *feed);
    void onFeedStatusChanged(Feed *sender);
    void onNetworkStateChanged();
};
}
#endif // FEEDCORE_SCHEDULER_H
