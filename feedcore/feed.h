/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "articleref.h"
#include "future.h"
#include <QDateTime>
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include <memory>

namespace FeedCore
{
/**
 * Abstract class for stored feeds.
 */
class Feed : public QObject
{
    Q_OBJECT

    /**
     * The user-facing display name of the feed.
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);

    /**
     * A category string used to group feeds together in the feed list.
     */
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged);

    /**
     * The url of the feed source.
     */
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged);

    /**
     * A link to a web page associated with this feed
     */
    Q_PROPERTY(QUrl link READ link WRITE setLink NOTIFY linkChanged);

    /**
     * A URL pointing to an image that will be displayed as the feed's icon
     */
    Q_PROPERTY(QUrl icon READ icon WRITE setIcon NOTIFY iconChanged);

    /**
     * The number of unread articles associated with this feed.
     */
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);

    /**
     * The feed's update status
     */
    Q_PROPERTY(FeedCore::Feed::LoadStatus status READ status NOTIFY statusChanged);

    /**
     * The time of the last update
     */
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)

    /**
     * Mode for determining when to automatically update the feed
     */
    Q_PROPERTY(FeedCore::Feed::UpdateMode updateMode READ updateMode WRITE setUpdateMode NOTIFY updateModeChanged)

    /**
     * How often to update the feed.
     *
     * This is normally set by the context unless updateMode is set to OverrideUpdateMode.
     */
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)

    /**
     * Mode for determining when to delete old items.
     */
    Q_PROPERTY(FeedCore::Feed::UpdateMode expireMode READ expireMode WRITE setExpireMode NOTIFY expireModeChanged)

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

    /**
     * The Updater instance that should be used to update this feed.
     */
    Q_PROPERTY(FeedCore::Feed::Updater *updater READ updater CONSTANT);

    /**
     * Whether to enable feed editing in the UI.  An editable feed implementation is expected
     * to monitor property changes and propagate them to the storage backend.
     *
     * The default implementation returns false; override and return true to enable editing.
     */
    Q_PROPERTY(bool editable READ editable CONSTANT);

public:
    class Updater;

    enum LoadStatus {
        Idle, /** < no active updates */
        Loading, /** < feed is being loaded from the storage backend */
        Updating, /** < feed is being updated from the source URL */
        Error /** < the last attempted update failed */
    };
    Q_ENUM(LoadStatus);

    enum UpdateMode {
        InheritUpdateMode, /** < Use update parameters provided by the context */
        OverrideUpdateMode, /** < Use update parameters specified by the feed */
        DisableUpdateMode, /** < Disable automatic updates */
    };
    Q_ENUM(UpdateMode)

    enum FeedFlags {
        UseReadableContentFlag = 1 /** < always use web content from readability */
    };
    Q_FLAGS(FeedFlags)

    ~Feed();

    /**
     * Returns a future representing all of the stored articles associated with this feed.
     *
     * If unreadFilter is true, only unread articles are returned.
     */
    virtual QFuture<ArticleRef> getArticles(bool unreadFilter) = 0;

    virtual Updater *updater() = 0;

    virtual bool editable();

    /**
     * Set this feed's metadata to match that of /other/
     */
    void updateParams(Feed *other);

    /**
     * Request that the feed be deleted from the storage backend.
     *
     * If the delete operation succeeds, the Feed object will be
     * destroyed, possibly asynchronously.  Connect to the QObject::destroyed
     * signal on the feed to handle this.
     */
    Q_INVOKABLE virtual void requestDelete();

    const QString &name() const;
    void setName(const QString &name);
    QString category() const;
    void setCategory(const QString &category);
    const QUrl &url() const;
    void setUrl(const QUrl &url);
    const QUrl &link();
    void setLink(const QUrl &link);
    const QUrl &icon();
    void setIcon(const QUrl &icon);
    int unreadCount() const;
    LoadStatus status() const;
    const QDateTime &lastUpdate();
    void setLastUpdate(const QDateTime &lastUpdate);
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
     * Emitted when an article has been added to the feed.
     */
    void articleAdded(const FeedCore::ArticleRef &article);

    /**
     * Emitted when a feed has changed significantly (e.g. when a
     * ProvisionalFeed is pointed at a new source). Code that
     * depends on the state of the Feed object should re-sync.
     */
    void reset();

    /**
     * Emitted when a feed is requested to be deleted.  The owner
     * of a feed should connect to this signal if it supports deleting.
     * If the delete succeeds, the reciever should ensure that the feed
     * object is destroyed.
     */
    void deleteRequested();

    void nameChanged();
    void categoryChanged();
    void urlChanged();
    void linkChanged();
    void iconChanged();
    void unreadCountChanged(int delta);
    void statusChanged();
    void lastUpdateChanged();
    void updateModeChanged();
    void updateIntervalChanged();
    void expireModeChanged();
    void expireAgeChanged();
    void flagsChanged();

protected:
    explicit Feed(QObject *parent = nullptr);
    void setUnreadCount(int unreadCount);
    void incrementUnreadCount(int delta = 1);
    void decrementUnreadCount();
    void setStatus(LoadStatus status);

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    friend Updater;
};

/**
 * Abstract class for updating feeds.
 *
 *  Derived classes implement run() to provide update logic
 */
class Feed::Updater : public QObject
{
    Q_OBJECT
public:
    Updater(Feed *feed, QObject *parent);
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
     * The feed that this updater belongs to
     */
    Feed *feed();

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
};

typedef Feed::LoadStatus LoadStatus;

}
