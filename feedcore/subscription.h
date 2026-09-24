/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "feed.h"

namespace FeedCore
{
/**
 * Abstract class for feeds that the user has subscribed to.
 */
class Subscription : public Feed
{
    Q_OBJECT

    /**
     * The url of the feed source.
     */
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged);

    /**
     * A link to a web page associated with this feed
     */
    Q_PROPERTY(QUrl link READ link WRITE setLink NOTIFY linkChanged);

    /**
     * Mode for determining when to automatically update the feed
     */
    Q_PROPERTY(FeedCore::Subscription::UpdateMode updateMode READ updateMode WRITE setUpdateMode NOTIFY updateModeChanged)

    /**
     * How often to update the feed.
     *
     * This is normally set by the context unless updateMode is set to OverrideUpdateMode.
     */
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)

    /**
     * Mode for determining when to delete old items.
     */
    Q_PROPERTY(FeedCore::Subscription::UpdateMode expireMode READ expireMode WRITE setExpireMode NOTIFY expireModeChanged)

    /**
     * Threshold for when items are considered stale and can be deleted.
     *
     * This is normally set by the context unless expireMode is set to OverrideUpdateMode.
     */
    Q_PROPERTY(int expireAge READ expireAge WRITE setExpireAge NOTIFY expireAgeChanged)

    /**
     * A mask containing the options for this feed.
     *
     * Available flags are found in the FeedFlags enum.
     */
    Q_PROPERTY(int flags READ flags WRITE setFlags NOTIFY flagsChanged)

public:
    class Updater;

    enum UpdateMode {
        InheritUpdateMode, /** < Use update parameters provided by the context */
        OverrideUpdateMode, /** < Use update parameters specified by the feed */
        DisableUpdateMode, /** < Disable automatic updates */
    };
    Q_ENUM(UpdateMode)

    enum FeedFlags {
        UseReadableContentFlag = 1, /** < always use web content from readability */
        IsWebPageFlag = 1U << 1U /** < this feed is a web page, not an RSS/Atom feed */
    };
    Q_FLAGS(FeedFlags)

    ~Subscription();

    /**
     * The Updater instance that should be used to update this subscription.
     */
    virtual Updater *updater() = 0;

    /**
     * Start an update using this subscription's Updater.
     */
    void update(const QDateTime &timestamp = QDateTime::currentDateTime()) final;

    /**
     * Abort the update in progress, if any, using this subscription's Updater.
     */
    void cancelUpdates() final;

    /**
     * Set this subscription's metadata to match that of /other/
     */
    void updateParams(Subscription *other);

    /**
     * Request that the subscription be deleted from the storage backend.
     *
     * If the delete operation succeeds, the Subscription object will be
     * destroyed, possibly asynchronously.  Connect to the QObject::destroyed
     * signal on the subscription to handle this.
     */
    Q_INVOKABLE virtual void requestDelete();

    const QUrl &url() const;
    void setUrl(const QUrl &url);
    const QUrl &link();
    void setLink(const QUrl &link);
    UpdateMode updateMode();
    void setUpdateMode(UpdateMode updateMode);
    qint64 updateInterval();
    void setUpdateInterval(qint64 updateInterval);
    UpdateMode expireMode();
    void setExpireMode(UpdateMode expireMode);
    void setExpireAge(qint64 expireAge);
    qint64 expireAge();
    void setFlags(int flags);
    int flags() const;

signals:
    /**
     * Emitted when a subscription is requested to be deleted.  The owner
     * of a subscription should connect to this signal if it supports deleting.
     * If the delete succeeds, the reciever should ensure that the subscription
     * object is destroyed.
     */
    void deleteRequested();

    void urlChanged();
    void linkChanged();
    void updateModeChanged();
    void updateIntervalChanged();
    void expireModeChanged();
    void expireAgeChanged();
    void flagsChanged();

protected:
    explicit Subscription(QObject *parent = nullptr);

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
};

/**
 * Abstract class for updating subscriptions.
 *
 *  Derived classes implement run() to provide update logic
 */
class Subscription::Updater : public QObject
{
    Q_OBJECT
public:
    Updater(Subscription *feed, QObject *parent);
    ~Updater();

    /**
     * Implemented by derived classes to abort the update.
     *
     * The implementation should call aborted() if the abort is successful.
     */
    Q_INVOKABLE virtual void abort(){};

    /**
     * Begin an update.
     *
     * This sets the feed status and records the update time, then calls run() to perform the actual update.
     */
    Q_INVOKABLE void start(const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * The last error reported by the implementation.
     *
     * This should not be used to determine whether an error has occured; use feed()->status() for that.
     */
    QString error();

    /**
     * The subscription that this updater belongs to
     */
    Subscription *feed();

    /**
     * If an update is in progress, the time that the update started,
     * otherwise an invalid QDateTime.
     */
    const QDateTime &updateStartTime();

protected:
    /**
     * Called by implemetations when an update completes successfuly.
     *
     * This should *not* be called when an error has occurred.
     */
    void finish();

    /**
     * Called by implementations when an update fails with an error
     */
    void setError(const QString &errorMsg);

    /**
     *  Called by implementations when an update is aborted
     */
    void aborted();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    /**
     * Implemented by derived classes to perform the update.
     */
    virtual void run() = 0;

    /**
     * Implemented by derived classes to clean up after an update.
     *
     * This is called at the end of an update whether is succeeded, failed, or was aborted.
     */
    virtual void cleanup();
};

}
