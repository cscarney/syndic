/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_CONTEXT_H
#define FEEDCORE_CONTEXT_H
#include <memory>
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include "future.h"

namespace FeedCore {
class Storage;
class Feed;
class ProvisionalFeed;

/**
 * A Context object represents an entire collection of feeds.
 */
class Context : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 defaultUpdateInterval READ defaultUpdateInterval WRITE setDefaultUpdateInterval NOTIFY defaultUpdateIntervalChanged)
    Q_PROPERTY(qint64 expireAge READ expireAge WRITE setExpireAge NOTIFY expireAgeChanged)
public:
    /**
     *  Create a context from a storage backend.
     *
     *  The newly-created context takes ownership of the Storage object.
     */
    explicit Context(Storage *storage, QObject *parent = nullptr);

    ~Context();

    /**
     * Request a list of all of the feeds in this context.  The resulting
     * feed objects are unique-per-feed and parented to the context's
     * storage backend.
     *
     * @return A future representing the list of feeds.
     */
    Future<Feed*> *getFeeds();

    /**
     * Create a new feed in the Context's storage object.
     *
     * The feed supplied to this function will usually be a ProvisionalFeed.  The created
     * Feed will be a duplicate of the one you provide; if you need access to the created
     * Feed object, you should to listen for the feedAdded signal, which will be emitted
     * asynchronously.
     */
    Q_INVOKABLE void addFeed(FeedCore::Feed *feed);

    /**
     * List articles stored in this context.
     *
     * This returns articles from all feeds; to get articles from a single feed use Feed::getArticles.
     *
     * If unreadFilter is true, returns only unread articles.
     *
     * @return A future representing the list of articles.
     */
    Future<ArticleRef> *getArticles(bool unreadFilter);

    /**
     * List starred articles stored in this context (isStarred == true).
     * @return A future representing the list of articles
     */
    Future<ArticleRef> *getStarred();

    /**
     * Trigger an update on every feed in this context.
     *
     * The updates may not succeed and this method does not provide
     * feedback.  If you need to track the status of the update you should
     * use an AllItemsFeed instance.
     */
    Q_INVOKABLE void requestUpdate();

    /**
     * Attempt to cancel updates for all feeds.
     *
     * The operation may not succeed and this method does not provide
     * feedback.  If you need to track the status of the update you should
     * use an AllItemsFeed instance.
     */
    void abortUpdates();

    /**
     * The update interval that will be used for UpdateMode::DefaultUpdateMode
     * @return The interval in seconds
     */
    qint64 defaultUpdateInterval();

    /**
     * Set the update interval (in seconds) that will be used for UpdateMode::DefaultUpdateMode
     */
    void setDefaultUpdateInterval(qint64 defaultUpdateInterval);

    /**
     * The age when articles are considered stale.
     * @return age in seconds
     */
    qint64 expireAge();

    /**
     * Set the age (in seconds) when articles are considered stale.
     *
     * Changing this value does not remove any stale articles; stale articles
     * will be deleted as part of the next update.  Setting this value to 0
     * disables item expiration.
     */
    void setExpireAge(qint64 expireAge);

signals:
    void defaultUpdateIntervalChanged();
    void expireAgeChanged();

    /**
     * Emitted when a newly-created feed is added.
     *
     * The supplied feed object is unique-per-feed and belongs to the feed's storage backend.
     */
    void feedAdded(FeedCore::Feed *feed);
private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
};
}
#endif // FEEDCORE_CONTEXT_H
