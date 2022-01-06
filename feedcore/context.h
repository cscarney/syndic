/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_CONTEXT_H
#define FEEDCORE_CONTEXT_H
#include "future.h"
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include <memory>

namespace FeedCore
{
class Storage;
class Feed;
class ProvisionalFeed;

/**
 * A Context object represents an entire collection of feeds.
 */
class Context : public QObject
{
    Q_OBJECT

    /**
     * Whether to schedule updates for feeds using UpdateMode::DefaultUpdateMode.
     *
     * The default value is false.  Set defaultUpdateInterval to a sane value before setting this to true.
     */
    Q_PROPERTY(bool defaultUpdateEnabled READ defaultUpdateEnabled WRITE setDefaultUpdateEnabled NOTIFY defaultUpdateEnabledChanged)

    /**
     * The update interval (in seconds) that will be used for UpdateMode::DefaultUpdateMode.
     *
     * The default is 0.
     */
    Q_PROPERTY(qint64 defaultUpdateInterval READ defaultUpdateInterval WRITE setDefaultUpdateInterval NOTIFY defaultUpdateIntervalChanged)

    /**
     * The age (in seconds) when articles are considered stale.
     *
     * Changing this value does not remove any stale articles; stale articles
     * will be deleted as part of the next update.  Setting this value to 0
     * disables item expiration.  The default is 0.
     */
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
     * In a newly created context, this will return an empty set, since feeds
     * are asynchronously loaded.  To get a complete feed list on startup,
     * you should wait for the feedListPopulated signal before calling this
     * function.
     */
    const QSet<Feed *> &getFeeds();

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
     * Get a list of category strings
     *
     * Returns an alphabetized list containing every category name that is assigned
     * to at least one feed.
     */
    Q_INVOKABLE QStringList getCategories();

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
     * Write an OPML document with the feeds stored in this context
     * to the provided url.  The URL must point to a writable local file.
     */
    Q_INVOKABLE void exportOpml(const QUrl &url) const;

    /**
     * Read an OPML document from the provided URL and add all of
     * the feeds in the file to this context.  The URL must point to a
     * local file.
     */
    Q_INVOKABLE void importOpml(const QUrl &url);

    bool defaultUpdateEnabled() const;
    void setDefaultUpdateEnabled(bool defaultUpdateEnabled);
    qint64 defaultUpdateInterval();
    void setDefaultUpdateInterval(qint64 defaultUpdateInterval);
    qint64 expireAge();
    void setExpireAge(qint64 expireAge);

signals:
    void defaultUpdateEnabledChanged();
    void defaultUpdateIntervalChanged();
    void expireAgeChanged();

    /**
     * Emitted when a feed is added to the context.  This may be a newly-created
     * feed, or a feed asynchronously loaded from storage.
     *
     * The supplied feed object is unique-per-feed and belongs to the feed's storage backend.
     */
    void feedAdded(FeedCore::Feed *feed);

    /**
     * Emitted once when the context finishes populating the feed list.
     *
     * After this signal is emitted, getFeeds() will include all stored feeds.
     */
    void feedListPopulated();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    void populateFeeds(const QVector<Feed *> &feeds);
};
}
#endif // FEEDCORE_CONTEXT_H
