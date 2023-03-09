/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "aggregatefeed.h"
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
class Readability;

class Context;

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

    Q_PROPERTY(bool prefetchContent READ prefetchContent WRITE setPrefetchContent NOTIFY prefetchContentChanged)

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
     * Returns a shared instance of AllItemsFeed for this context.
     *
     * The resulting instance is created the first time the function
     * is called and is owned by the context. Using the shared instance
     * is preferred if you don't need custom settings for the feed
     * (name, etc.).
     */
    QSharedPointer<Feed> allItemsFeed();

    /**
     * Request a filtered set of feeds matching the given category.
     *
     * The resulting set is generated on each call, so the caller should
     * cache it when possible.
     */
    QSet<Feed *> getCategoryFeeds(const QString &category);

    /**
     * Create a new category feed.
     *
     * The caller assumes ownership of the returned object. This function
     * is primarily meant to be called from QML. From C++, it's preferable
     * to create a FeedCore::CategoryFeed directly.
     */
    Q_INVOKABLE FeedCore::Feed *createCategoryFeed(const QString &category);

    /**
     * Create a new feed in the Context's storage object.
     *
     * The feed supplied to this function will usually be a ProvisionalFeed.  The created
     * Feed will be a duplicate of the one you provide; if you need access to the created
     * Feed object, you should to listen for ProvisionalFeed::targetFeedChanged and
     * ProvisionalFeed::saveError.
     */
    Q_INVOKABLE void addFeed(FeedCore::ProvisionalFeed *feed);

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

    Readability *getReadability();

    bool defaultUpdateEnabled() const;
    void setDefaultUpdateEnabled(bool defaultUpdateEnabled);
    qint64 defaultUpdateInterval();
    void setDefaultUpdateInterval(qint64 defaultUpdateInterval);
    qint64 expireAge();
    void setExpireAge(qint64 expireAge);
    bool prefetchContent() const;
    void setPrefetchContent(bool newPrefetchContent);

signals:
    void defaultUpdateEnabledChanged();
    void defaultUpdateIntervalChanged();
    void expireAgeChanged();
    void prefetchContentChanged();

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
    void feedListPopulated(int nFeeds);

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    void populateFeeds(const QVector<Feed *> &feeds);
    void registerFeeds(const QVector<Feed *> &feeds);
};
}
