/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "sqlite/storageimpl.h"
#include "articleref.h"
#include "provisionalfeed.h"
#include "sqlite/articleimpl.h"
#include "sqlite/feedimpl.h"
#include <QTimer>
#include <QVector>
#include <Syndication/Person>
#include <utility>
using namespace FeedCore;
using namespace SqliteStorage;

void StorageImpl::appendArticleResults(Future<ArticleRef> *op, ItemQuery &q)
{
    while (q.next()) {
        const auto &feed = m_feedFactory.getInstance(q.feed(), this);
        const auto &item = m_articleFactory.getInstance(q.id(), this, feed, q);
        item->updateFromQuery(q);
        op->appendResult(item);
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

Future<ArticleRef> *StorageImpl::getAll()
{
    return Future<ArticleRef>::yield(this, [this](auto *op) {
        ItemQuery q{m_db.selectAllItems()};
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getUnread()
{
    return Future<ArticleRef>::yield(this, [this](auto *op) {
        ItemQuery q{m_db.selectUnreadItems()};
        appendArticleResults(op, q);
    });
}

FeedCore::Future<ArticleRef> *StorageImpl::getStarred()
{
    return Future<ArticleRef>::yield(this, [this](auto *op) {
        ItemQuery q{m_db.selectStarredItems()};
        appendArticleResults(op, q);
    });
}

StorageImpl::StorageImpl(const QString &filePath)
    : m_db(filePath)
{
}

Future<ArticleRef> *StorageImpl::getById(qint64 id)
{
    return Future<ArticleRef>::yield(this, [this, id](auto *op) {
        ItemQuery q{m_db.selectItem(id)};
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getByFeed(FeedImpl *feed)
{
    const qint64 feedId{feed->id()};
    return Future<ArticleRef>::yield(this, [this, feedId](auto *op) {
        ItemQuery q = m_db.selectItemsByFeed(feedId);
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getUnreadByFeed(FeedImpl *feed)
{
    const qint64 feedId = feed->id();
    return Future<ArticleRef>::yield(this, [this, feedId](auto *op) {
        ItemQuery q{m_db.selectUnreadItemsByFeed(feedId)};
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item)
{
    const qint64 feedId{feed->id()};
    return Future<ArticleRef>::yield(this, [this, item, feedId](auto *op) {
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

            // TODO this  sometimes creates an unnecessary item instance
            getById(*itemId); // push the update into any existing item instance

            op->setResult();
            return;
        }

        if (date == 0) {
            date = QDateTime::currentSecsSinceEpoch();
        }
        const auto &newId = m_db.insertItem(feedId, item->id(), item->title(), authorName, date, item->link(), content);
        if (!newId) {
            op->setResult();
            return;
        }
        ItemQuery result{m_db.selectItem(*newId)};
        appendArticleResults(op, result);
    });
}

FeedCore::Future<QString> *StorageImpl::getContent(ArticleImpl *article)
{
    qint64 id = article->id();
    return Future<QString>::yield(this, [this, id](auto *op) {
        op->appendResult(m_db.selectItemContent(id));
    });
}

void StorageImpl::onArticleReadChanged(ArticleImpl *article)
{
    const qint64 itemId{article->id()};
    const bool isRead{article->isRead()};
    m_db.updateItemRead(itemId, isRead);
}

void StorageImpl::onArticleStarredChanged(ArticleImpl *article)
{
    const qint64 itemId{article->id()};
    const bool isStarred{article->isStarred()};
    m_db.updateItemStarred(itemId, isStarred);
}

void StorageImpl::appendFeedResults(Future<Feed *> *op, FeedQuery &q)
{
    while (q.next()) {
        auto *ref = m_feedFactory.getInstance(q.id(), this);
        ref->updateFromQuery(q);
        op->appendResult(ref);
    }
}

Future<Feed *> *StorageImpl::getFeeds()
{
    return Future<Feed *>::yield(this, [this](auto *op) {
        FeedQuery q{m_db.selectAllFeeds()};
        appendFeedResults(op, q);
    });
}

static qint64 packFeedUpdateInterval(Feed *feed)
{
    switch (feed->updateMode()) {
    case Feed::DefaultUpdateMode:
    default:
        return 0;

    case Feed::ManualUpdateMode:
        return -1;

    case Feed::CustomUpdateMode:
        return feed->updateInterval();
    }
}

Future<Feed *> *StorageImpl::storeFeed(Feed *feed)
{
    const QUrl &url = feed->url();
    const QString &name = feed->name();
    const QString &category = feed->category();
    const qint64 updateInterval = packFeedUpdateInterval(feed);
    return Future<Feed *>::yield(this, [this, url, name, category, updateInterval](auto *op) {
        const auto &insertId = m_db.insertFeed(url);
        if (!insertId) {
            op->setResult();
            return;
        }
        m_db.updateFeedUpdateInterval(*insertId, updateInterval);
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
    if (feed->updateMode() != Feed::CustomUpdateMode) {
        return;
    }
    db.updateFeedUpdateInterval(feedId, feed->updateInterval());
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
    QObject::connect(feed, &Feed::nameChanged, this, [this, feed] {
        m_db.updateFeedName(feed->id(), feed->name());
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
    QObject::connect(feed, &Feed::deleteRequested, this, [this, feed] {
        onFeedRequestDelete(feed);
    });
}

void StorageImpl::expire(FeedImpl *feed, const QDateTime &olderThan)
{
    m_db.deleteItemsOlderThan(feed->id(), olderThan);
}
