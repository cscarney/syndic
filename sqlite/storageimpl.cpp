/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "sqlite/storageimpl.h"
#include "articleref.h"
#include "provisionalfeed.h"
#include "sqlite/articleimpl.h"
#include "sqlite/feedimpl.h"
#include <QCoreApplication>
#include <QEvent>
#include <QTimer>
#include <QVector>
#include <Syndication/Person>
#include <utility>
using namespace FeedCore;
using namespace SqliteStorage;

const int StorageImpl::CommitEvent = QEvent::registerEventType();

void StorageImpl::appendArticleResults(QPromise<ArticleRef> &op, ItemQuery &q)
{
    while (q.next()) {
        const auto &feed = m_feedFactory.getInstance(q.feed(), this);
        const auto &item = m_articleFactory.getInstance(q.id(), this, feed, q);
        item->updateFromQuery(q);
        op.addResult(item);
    }
}

void StorageImpl::onFeedRequestDelete(FeedImpl *feed)
{
    feed->updater()->abort();
    qint64 feedId{feed->id()};
    m_db.deleteItemsForFeed(feedId);
    m_db.deleteFeed(feedId);
    feed->deleteLater();
}

void StorageImpl::ensureTransaction()
{
    if (m_hasTransaction) {
        return;
    }
    m_db.beginTransaction();
    m_hasTransaction = true;
    QCoreApplication::postEvent(this, new QEvent(static_cast<QEvent::Type>(CommitEvent)), Qt::LowEventPriority);
}

QFuture<ArticleRef> StorageImpl::getAll()
{
    return Future::yield<ArticleRef>(this, [this](auto &op) {
        ItemQuery q{m_db.selectAllItems()};
        appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getUnread()
{
    return Future::yield<ArticleRef>(this, [this](auto &op) {
        ItemQuery q{m_db.selectUnreadItems()};
        appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getStarred()
{
    return Future::yield<ArticleRef>(this, [this](auto &op) {
        ItemQuery q{m_db.selectStarredItems()};
        appendArticleResults(op, q);
    });
}

StorageImpl::StorageImpl(const QString &filePath)
    : m_db(filePath)
{
}

StorageImpl::~StorageImpl()
{
    if (m_hasTransaction) {
        m_db.commitTransaction();
    }
}

QFuture<ArticleRef> StorageImpl::getById(qint64 id)
{
    return Future::yield<ArticleRef>(this, [this, id](auto &op) {
        ItemQuery q{m_db.selectItem(id)};
        appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getByFeed(FeedImpl *feed)
{
    const qint64 feedId{feed->id()};
    return Future::yield<ArticleRef>(this, [this, feedId](auto &op) {
        ItemQuery q = m_db.selectItemsByFeed(feedId);
        appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getUnreadByFeed(FeedImpl *feed)
{
    const qint64 feedId = feed->id();
    return Future::yield<ArticleRef>(this, [this, feedId](auto &op) {
        ItemQuery q{m_db.selectUnreadItemsByFeed(feedId)};
        appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item)
{
    const qint64 feedId{feed->id()};
    return runInTransaction<ArticleRef>([this, item, feedId](auto &op) {
        const auto &itemId = m_db.selectItemId(feedId, item->id());
        const auto &authors = item->authors();
        const auto &authorName = authors.empty() ? "" : authors[0]->name();
        time_t date = item->dateUpdated();
        auto content = item->content();
        if (content.isEmpty()) {
            content = item->description();
        }

        if (itemId) {
            m_db.updateItemHeaders(*itemId, item->title(), authorName, item->link());
            if (date > 0) {
                m_db.updateItemDate(*itemId, date);
            }
            if (!content.isEmpty()) {
                m_db.updateItemContent(*itemId, content);
            }

            // Update the existing instance of the article
            if (m_articleFactory.hasInstance(*itemId)) {
                getById(*itemId);
            }
            return;
        }

        if (date == 0) {
            date = QDateTime::currentSecsSinceEpoch();
        }
        const auto &newId = m_db.insertItem(feedId, item->id(), item->title(), authorName, date, item->link(), content);
        if (!newId) {
            return;
        }
        ItemQuery result{m_db.selectItem(*newId)};
        appendArticleResults(op, result);
    });
}

QFuture<QString> StorageImpl::getContent(ArticleImpl *article)
{
    qint64 id = article->id();
    return Future::yield<QString>(this, [this, id](auto &op) {
        op.addResult(m_db.selectItemContent(id));
    });
}

QFuture<QString> StorageImpl::getReadableContent(ArticleImpl *article)
{
    qint64 id = article->id();
    return Future::yield<QString>(this, [this, id](auto &op) {
        QString readableContent = m_db.selectItemReadableContent(id);
        if (!readableContent.isEmpty()) {
            op.addResult(readableContent);
        }
    });
}

void StorageImpl::cacheReadableContent(ArticleImpl *article, const QString &readableContent)
{
    const qint64 itemId{article->id()};
    ensureTransaction();
    m_db.updateItemReadableContent(itemId, readableContent);
}

void StorageImpl::onArticleReadChanged(ArticleImpl *article)
{
    const qint64 itemId{article->id()};
    const bool isRead{article->isRead()};
    ensureTransaction();
    m_db.updateItemRead(itemId, isRead);
}

void StorageImpl::onArticleStarredChanged(ArticleImpl *article)
{
    const qint64 itemId{article->id()};
    const bool isStarred{article->isStarred()};
    ensureTransaction();
    m_db.updateItemStarred(itemId, isStarred);
}

void StorageImpl::appendFeedResults(QPromise<Feed *> &op, FeedQuery &q)
{
    while (q.next()) {
        auto *ref = m_feedFactory.getInstance(q.id(), this);
        ref->updateFromQuery(q);
        op.addResult(ref);
    }
}

QFuture<Feed *> StorageImpl::getFeeds()
{
    return Future::yield<Feed *>(this, [this](auto &op) {
        FeedQuery q{m_db.selectAllFeeds()};
        appendFeedResults(op, q);
    });
}

static qint64 packModeValue(Feed::UpdateMode mode, qint64 value)
{
    switch (mode) {
    case Feed::InheritUpdateMode:
    default:
        return 0;

    case Feed::DisableUpdateMode:
        return -1;

    case Feed::OverrideUpdateMode:
        return value;
    }
}

static qint64 packFeedUpdateInterval(Feed *feed)
{
    return packModeValue(feed->updateMode(), feed->updateInterval());
}

static qint64 packFeedExpireAge(Feed *feed)
{
    return packModeValue(feed->expireMode(), feed->expireAge());
}

QFuture<Feed *> StorageImpl::storeFeed(Feed *feed)
{
    const QUrl &url = feed->url();
    const QString &name = feed->name();
    const QString &category = feed->category();
    const qint64 updateInterval = packFeedUpdateInterval(feed);
    const qint64 expireAge = packFeedExpireAge(feed);
    return runInTransaction<FeedCore::Feed *>([this, url, name, category, updateInterval, expireAge](auto &op) {
        const auto &insertId = m_db.insertFeed(url);
        if (!insertId) {
            return;
        }
        m_db.updateFeedUpdateInterval(*insertId, updateInterval);
        m_db.updateFeedExpireAge(*insertId, expireAge);
        m_db.updateFeedName(*insertId, name);
        m_db.updateFeedCategory(*insertId, category);
        FeedQuery result{m_db.selectFeed(*insertId)};
        appendFeedResults(op, result);
    });
}

static void onUpdateModeChanged(FeedDatabase &db, Feed *feed, qint64 feedId)
{
    qint64 updateInterval = packFeedUpdateInterval(feed);
    db.updateFeedUpdateInterval(feedId, updateInterval);
}

static void onUpdateIntervalChanged(FeedDatabase &db, Feed *feed, qint64 feedId)
{
    if (feed->updateMode() != Feed::OverrideUpdateMode) {
        return;
    }
    db.updateFeedUpdateInterval(feedId, feed->updateInterval());
}

static void onExpireModeChanged(FeedDatabase &db, FeedImpl *feed)
{
    qint64 expireAge = packFeedExpireAge(feed);
    db.updateFeedExpireAge(feed->id(), expireAge);
}

static void onExpireAgeChanged(FeedDatabase &db, FeedImpl *feed)
{
    if (feed->expireMode() != Feed::OverrideUpdateMode) {
        return;
    }
    db.updateFeedExpireAge(feed->id(), feed->expireAge());
}

void StorageImpl::listenForChanges(FeedImpl *feed)
{
    qint64 feedId = feed->id();
    QObject::connect(feed, &Feed::lastUpdateChanged, this, [this, feed, feedId] {
        m_db.updateFeedLastUpdate(feedId, feed->lastUpdate());
    });
    QObject::connect(feed, &Feed::updateIntervalChanged, this, [this, feed, feedId] {
        onUpdateIntervalChanged(m_db, feed, feedId);
    });
    QObject::connect(feed, &Feed::updateModeChanged, this, [this, feed, feedId] {
        onUpdateModeChanged(m_db, feed, feedId);
    });
    QObject::connect(feed, &Feed::expireModeChanged, this, [this, feed] {
        onExpireModeChanged(m_db, feed);
    });
    QObject::connect(feed, &Feed::expireAgeChanged, this, [this, feed] {
        onExpireAgeChanged(m_db, feed);
    });
    QObject::connect(feed, &Feed::nameChanged, this, [this, feed] {
        m_db.updateFeedName(feed->id(), feed->name());
    });
    QObject::connect(feed, &Feed::urlChanged, this, [this, feed] {
        m_db.updateFeedUrl(feed->id(), feed->url());
    });
    QObject::connect(feed, &Feed::categoryChanged, this, [this, feed] {
        m_db.updateFeedCategory(feed->id(), feed->category());
    });
    QObject::connect(feed, &Feed::linkChanged, this, [this, feed] {
        m_db.updateFeedLink(feed->id(), feed->link().toString());
    });
    QObject::connect(feed, &Feed::iconChanged, this, [this, feed] {
        m_db.updateFeedIcon(feed->id(), feed->icon().toString());
    });
    QObject::connect(feed, &Feed::flagsChanged, this, [this, feed] {
        m_db.updateFeedFlags(feed->id(), feed->flags());
    });
    QObject::connect(feed, &Feed::deleteRequested, this, [this, feed] {
        onFeedRequestDelete(feed);
    });
}

void StorageImpl::expire(FeedImpl *feed, const QDateTime &olderThan)
{
    m_db.deleteItemsOlderThan(feed->id(), olderThan);
}

void StorageImpl::customEvent(QEvent *e)
{
    if (e->type() == static_cast<int>(CommitEvent)) {
        m_db.commitTransaction();
        m_hasTransaction = false;
        e->accept();
    } else {
        FeedCore::Storage::customEvent(e);
    }
}
