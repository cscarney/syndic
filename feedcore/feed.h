/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_FEED_H
#define FEEDCORE_FEED_H
#include "future.h"
#include <QDateTime>
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include <memory>

namespace FeedCore
{
/**
 * Abstract class for stored feeds
 */
class Feed : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged);
    Q_PROPERTY(QUrl link READ link WRITE setLink NOTIFY linkChanged);
    Q_PROPERTY(QUrl icon READ icon WRITE setIcon NOTIFY iconChanged);
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);
    Q_PROPERTY(FeedCore::Feed::LoadStatus status READ status NOTIFY statusChanged);
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)
    Q_PROPERTY(FeedCore::Feed::UpdateMode updateMode READ updateMode WRITE setUpdateMode NOTIFY updateModeChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(FeedCore::Feed::Updater *updater READ updater CONSTANT);
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
        DefaultUpdateMode, /** < Use the default update interval */
        CustomUpdateMode, /** < Use the update interval provided by the updateInterval property */
        ManualUpdateMode, /** < Do not perform scheduled updates */
    };
    Q_ENUM(UpdateMode)

    ~Feed();
    /**
     * The user-facing display name of the feed.
     */
    const QString &name() const;

    /**
     * Set the name of the feed.
     */
    void setName(const QString &name);

    /**
     * The url of the feed source.
     */
    const QUrl &url() const;

    /**
     * Set the url of the feed source.
     */
    void setUrl(const QUrl &url);

    /**
     * A link to a web page associated with this feed
     */
    const QUrl &link();

    /**
     * Set the web link associated associated with this feed
     */
    void setLink(const QUrl &link);

    /**
     * A URL pointing to an image that will be displayed as the feed's icon
     */
    const QUrl &icon();

    /**
     * Set the icon URL.
     */
    void setIcon(const QUrl &icon);

    /**
     * The number of unread articles associated with this feed.
     */
    int unreadCount() const;

    /**
     * The feed's update status
     */
    LoadStatus status() const;

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

    qint64 expireAge();

    /**
     * Set this feed's metadata to match that of /other/
     */
    void updateParams(Feed *other);

    /**
     * The Updater instance that should be used to update this feed.
     *
     * The returned updater belongs to the Feed object.
     */
    virtual Updater *updater() = 0;

    /**
     * Returns a future representing all of the stored articles associated with this feed.
     *
     * If unreadFilter is true, only unread articles are returned.
     */
    virtual Future<ArticleRef> *getArticles(bool unreadFilter) = 0;

    /**
     * Returns true if the implementation supports storing and propagating
     * changes to the feed's properties.
     */
    virtual bool editable();

    /**
     * Request that the feed be deleted from the storage backend.
     *
     * If the delete operation succeeds, the Feed object will be
     * destroyed, possibly asynchronously.  Connect to the QObject::destroyed
     * signal on the feed to handle this.
     */
    Q_INVOKABLE virtual void requestDelete();

signals:
    void nameChanged();
    void urlChanged();
    void linkChanged();
    void iconChanged();
    void unreadCountChanged(int delta);
    void statusChanged();
    void lastUpdateChanged();
    void updateModeChanged();
    void updateIntervalChanged();

    /**
     * Emitted when old items should be removed from the storage backend
     */
    void expire(const QDateTime &olderThan);

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

protected:
    explicit Feed(QObject *parent = nullptr);
    void setUnreadCount(int unreadCount);
    void incrementUnreadCount(int delta = 1);
    void decrementUnreadCount();
    ;
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
    virtual void abort(){};

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
#endif // FEEDCORE_FEED_H
