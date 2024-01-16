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

    /* whether feeds should preload readable content during feed updates */
    Q_PROPERTY(bool prefetchContent READ prefetchContent WRITE setPrefetchContent NOTIFY prefetchContentChanged)

    /* whether the feed list has finished loading */
    Q_PROPERTY(bool feedListComplete READ feedListComplete NOTIFY feedListCompleteChanged)

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
     * are asynchronously loaded from storage.
     */
    const QSet<Feed *> &getFeeds();

    /**
     * Returns a shared instance of the "All Items" feed for this context.
     *
     * The resulting instance is created on-demand and shared among
     * all callers.
     *
     * The all items feed is an aggregate feed that contains all of the
     * articles from all of the feeds in this context. Updating the all
     * items feed will trigger updates on all of the feeds in the context.
     *
     * The status of the all items feed reflects the status of all of the
     * feeds in the context:
     *      Feed::Idle - all feeds have been loaded and all updates are complete
     *      Feed::Loading - feeds are still being loaded from storage
     *      Feed::Updating - one or more feeds are being updated
     *  Note that the all items feed does not currently track the error state
     *  of is component feeds; the status will never be Feed::Error even if
     *  there are individual feeds with errors.
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
     * Return the list of articles matching a search query
     */
    QFuture<ArticleRef> searchArticles(const QString &query);

    /**
     * Create a new feed in the Context's storage object.
     *
     * The pattern for adding a new feed is:
     *  - Create a new ProvisionalFeed object and set (at least) the URL property
     *  - Optionally update the ProvisionalFeed to populate the remaining feed properties
     *  - Pass the ProvisionalFeed to Context::addFeed
     *  - Listen for ProvisionalFeed::targetFeedChanged and ProvisionalFeed::saveFailed signals
     *  - Delete the ProvisionalFeed
     *
     * On success, ProvisionalFeed::targetFeed will be set to the newly created feed. On failure,
     * the ProvisionalFeed::saveFailed signal will be emitted.
     *
     * The newly created feed is owned by the context's storage object. Ownership of the
     * ProvisionalFeed remains with the caller. The ProvisionalFeed may be deleted immediately
     * after the addFeed call if the caller does not need feedback on the success of the
     * add operation.
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
     * List all articles from all feeds managed by this context.
     *
     * If unreadFilter is true, returns only unread articles.
     *
     * @return A future representing the list of articles.
     */
    QFuture<ArticleRef> getArticles(bool unreadFilter);

    /**
     * List starred articles stored in this context (isStarred == true).
     * @return A future representing the list of articles
     */
    QFuture<ArticleRef> getStarred();

    static constexpr const int kDefaultNumberOfRecommendedItems = 20;

    /**
     * List articles that may be of interest to the user.
     *
     * The specific algorithm is determined by the storage
     * backend.
     */
    QFuture<ArticleRef> getHighlights(size_t limit = kDefaultNumberOfRecommendedItems);

    /**
     * Trigger an update on every feed in this context.
     *
     * The updates may not succeed and this method does not provide
     * feedback. If you need to track the status of the update you should
     * use allItemsFeed().
     */
    Q_INVOKABLE void requestUpdate();

    /**
     * Attempt to cancel updates for all feeds.
     *
     * The operation may not succeed and this method does not provide
     * feedback. If you need to track the status of the update you should
     * use allItemsFeed().
     */
    void abortUpdates();

    /**
     * Write an OPML document with the feeds stored in this context
     * to the provided url. The URL must point to a writable local file.
     */
    Q_INVOKABLE void exportOpml(const QUrl &url) const;

    /**
     * Read an OPML document from the provided URL and add all of
     * the feeds in the file to this context. The URL must point to a
     * local file.
     */
    Q_INVOKABLE void importOpml(const QUrl &url);

    /**
     * Gets the Readability object for this context.
     *
     * This may be null if we were built without readability support
     */
    Readability *getReadability();

    bool defaultUpdateEnabled() const;
    void setDefaultUpdateEnabled(bool defaultUpdateEnabled);
    qint64 defaultUpdateInterval();
    void setDefaultUpdateInterval(qint64 defaultUpdateInterval);
    qint64 expireAge();
    void setExpireAge(qint64 expireAge);
    bool prefetchContent() const;
    void setPrefetchContent(bool newPrefetchContent);
    bool feedListComplete();

signals:
    void defaultUpdateEnabledChanged();
    void defaultUpdateIntervalChanged();
    void expireAgeChanged();
    void prefetchContentChanged();
    void feedListCompleteChanged();

    /**
     * Emitted when a feed is added to the context.  This may be a newly-created
     * feed, or a feed asynchronously loaded from storage.
     *
     * The supplied feed object is unique-per-feed and belongs to the feed's storage backend.
     */
    void feedAdded(FeedCore::Feed *feed);

    /**
     * Emitted when the user needs to be prompted to configure the context.
     *
     * This occurs when the context's storage has no configured feeds.
     */
    void firstRun();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    void populateFeeds(const QList<Feed *> &feeds);
    void registerFeeds(const QList<Feed *> &feeds);
    void setFeedListComplete(bool feedListComplete);
};
}
