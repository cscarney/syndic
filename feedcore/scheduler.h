/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "feed.h"
#include "future.h"
#include <QDateTime>
#include <QObject>
#include <memory>

namespace FeedCore
{
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
    void schedule(Feed *feed, const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * Remove a feed from the update schedule.
     */
    void unschedule(Feed *feedRef);

    static constexpr const int kDefaultTimerResolution = 60000;

    /**
     * Start the update timer
     *
     * Once this method is called, stale feeds will be checked for every /resolution/ msecs
     * until stop() is called.
     */
    void start(int resolution = kDefaultTimerResolution);

    /**
     * Stop the update timer
     */
    void stop();

    bool isRunning();

    /**
     * Perform a one-time update of all stale feeds.
     */
    void updateStale();

    /**
     * Retry any scheduled updates that failed for some reason
     */
    void clearErrors();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    void reschedule(Feed *feed, const QDateTime &timestamp = QDateTime::currentDateTime());
    void onUpdateModeChanged(Feed *feed);
    void onFeedStatusChanged(Feed *sender);
    void onNetworkStateChanged();
};
}
