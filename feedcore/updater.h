/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_UPDATER_H
#define FEEDCORE_UPDATER_H
#include <Syndication/Feed>
#include <QObject>
#include <QDateTime>
#include <memory>

namespace FeedCore {
class Feed;

/**
 * Abstract class for updating feeds.
 *
 * The base class contains code for tracking update times and determining when
 * updates are due; derived classes are responsible for performing the actual update.
 */
class Updater : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)
    Q_PROPERTY(FeedCore::Updater::UpdateMode updateMode READ updateMode WRITE setUpdateMode NOTIFY updateModeChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
public:
    enum UpdateMode {
        DefaultUpdateMode, /** < Use the default update interval */
        CustomUpdateMode, /** < Use the update interval provided by the updateInterval property */
        ManualUpdateMode, /** < Do not perform scheduled updates */
    };
    Q_ENUM(UpdateMode)

    Updater(Feed *feed, QObject *parent);
    ~Updater();

    /**
     * Implemented by derived classes to abort the update.
     */
    virtual void abort() = 0;

    /**
     * Begin an update.
     *
     * If the update succeeds, timestamp is recorded as the time of the update
     */
    Q_INVOKABLE void start(const QDateTime &timestamp=QDateTime::currentDateTime());

    /**
     * The last error reported by the implementation.
     *
     * This should not be used to determine whether an error has occured; use feed()->status() for that.
     */
    QString error();

    /**
     * The feed that this updater belongs to
     */
    Feed *feed();

    /**
     * The timestamp when the next update should occur.
     */
    QDateTime nextUpdate();

    bool hasNextUpdate();

    /**
     * Whether an update is due as of /timestamp/
     */
    bool needsUpdate(const QDateTime &timestamp);

    /**
     * Start an update if an update is due
     *
     * Timestamp is used as the current time for determining
     * whether to perform the update.  It will also be recorded
     * as the time of the update if it succeeds.
     */
    bool updateIfNecessary(const QDateTime &timestamp);

    /**
     * The time of the last update
     */
    const QDateTime &lastUpdate();

    /**
     * Set the timestamp of the last update.
     */
    void setLastUpdate(const QDateTime &lastUpdate);

    /**
     * The update scheduling mode
     */
    UpdateMode updateMode();

    /**
     * Set the update scheduling mode.
     */
    void setUpdateMode(UpdateMode updateMode);

    /**
     * How often to update the feed.
     */
    qint64 updateInterval();

    /**
     * Set the update interval.
     *
     * This value is used to determine when an update is due.
     */
    void setUpdateInterval(qint64 updateInterval);

    /**
     * Sets the update interval that will be used when using DefaultupdateMode
     */
    void setDefaultUpdateInterval(qint64 updateInterval);

    /**
     * Set the age when article are considered stale.
     *
     * A value of 0 disables stale item expiration.
     * Stale items will not be removed until the next update.
     */
    void setExpireAge(qint64 expireAge);

    /**
     * Set this Updater's parameters to match /other/
     */
    void updateParams(Updater *other);
signals:
    void lastUpdateChanged();
    void updateModeChanged();
    void updateIntervalChanged();
    void expire(const QDateTime &olderThan);
protected:
    void finish();
    void setError(const QString &errorMsg);
private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    /**
     * Implemented by derived classes to perform the update
     */
    virtual void run() = 0;
};
}

#endif // FEEDCORE_UPDATER_H
