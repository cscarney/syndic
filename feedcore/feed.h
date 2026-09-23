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
 * Abstract class for any list of articles that can be displayed as a feed.
 *
 * This includes both user subscriptions (see Subscription) and synthesized
 * feeds such as aggregates, starred items, and search results.
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

public:
    enum LoadStatus {
        Idle, /** < no active updates */
        Loading, /** < feed is being loaded from the storage backend */
        Updating, /** < feed is being updated from the source URL */
        Error /** < the last attempted update failed */
    };
    Q_ENUM(LoadStatus);

    ~Feed();

    /**
     * Returns a future representing all of the stored articles associated with this feed.
     *
     * If unreadFilter is true, only unread articles are returned.
     */
    virtual QFuture<ArticleRef> getArticles(bool unreadFilter) = 0;

    /**
     * Request that the feed update its content from its source(s).
     *
     * /timestamp/ is recorded as the time of the update.  Aggregate feeds pass
     * their timestamp on to their component feeds so that all of the updates
     * share the same time.
     *
     * Progress is reported through the status property.  The default
     * implementation does nothing.
     */
    Q_INVOKABLE virtual void update(const QDateTime &timestamp = QDateTime::currentDateTime());

    /**
     * Attempt to cancel any update in progress.
     *
     * The default implementation does nothing.
     */
    Q_INVOKABLE virtual void cancelUpdates();

    const QString &name() const;
    void setName(const QString &name);
    QString category() const;
    void setCategory(const QString &category);
    const QUrl &icon();
    void setIcon(const QUrl &icon);
    int unreadCount() const;
    LoadStatus status() const;
    const QDateTime &lastUpdate();
    void setLastUpdate(const QDateTime &lastUpdate);

signals:
    /**
     * Emitted when an article has been added to the feed.
     */
    void articleAdded(const FeedCore::ArticleRef &article);

    /**
     * Emitted when a feed has changed significantly (e.g. when a
     * ProvisionalSubscription is pointed at a new source). Code that
     * depends on the state of the Feed object should re-sync.
     */
    void reset();

    void nameChanged();
    void categoryChanged();
    void iconChanged();
    void unreadCountChanged(int delta);
    void statusChanged();
    void lastUpdateChanged();

protected:
    explicit Feed(QObject *parent = nullptr);
    void setUnreadCount(int unreadCount);
    void incrementUnreadCount(int delta = 1);
    void decrementUnreadCount();
    void setStatus(LoadStatus status);

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
};

typedef Feed::LoadStatus LoadStatus;

}
