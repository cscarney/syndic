﻿/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "sqlite/storageimpl.h"
#include <QVector>
#include <QTimer>
#include <Syndication/Person>
#include <utility>
#include "sqlite/feedimpl.h"
#include "sqlite/articleimpl.h"
#include "articleref.h"
#include "provisionalfeed.h"
#include "updater.h"
using namespace FeedCore;
using namespace Sqlite;

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
    qint64 feedId { feed->id() };
    m_db.deleteItemsForFeed(feedId);
    m_db.deleteFeed(feedId);
    feed->deleteLater();
}

Future<ArticleRef> *StorageImpl::getAll()
{
    return Future<ArticleRef>::yield(this, [this](auto *op){
        ItemQuery q { m_db.selectAllItems() };
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getUnread()
{
    return Future<ArticleRef>::yield(this, [this](auto *op){
        ItemQuery q { m_db.selectUnreadItems() };
        appendArticleResults(op, q);
    });
}

FeedCore::Future<ArticleRef> *StorageImpl::getStarred()
{
    return Future<ArticleRef>::yield(this, [this](auto *op){
        ItemQuery q { m_db.selectStarredItems() };
        appendArticleResults(op, q);
    });
}

StorageImpl::StorageImpl(const QString& filePath) :
    m_db(filePath)
{

}

Future<ArticleRef> *StorageImpl::getById(qint64 id)
{
    return Future<ArticleRef>::yield(this, [this, id](auto *op){
        ItemQuery q { m_db.selectItem(id) };
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getByFeed(FeedImpl *feed)
{
    const qint64 feedId { feed->id() };
    return Future<ArticleRef>::yield(this, [this, feedId](auto *op){
        ItemQuery q = m_db.selectItemsByFeed(feedId);
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getUnreadByFeed(FeedImpl *feed)
{
    const qint64 feedId = feed->id();
    return Future<ArticleRef>::yield(this, [this, feedId](auto *op){
        ItemQuery q { m_db.selectUnreadItemsByFeed(feedId) };
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item)
{
    const qint64 feedId { feed->id() };
    return Future<ArticleRef>::yield(this, [this, item, feedId](auto *op){
        const auto &itemId = m_db.selectItemId(feedId, item->id());
        const auto &authors = item->authors();
        const auto &authorName = authors.empty() ? "" : authors[0]->name();
        const auto &date = QDateTime::fromTime_t(item->dateUpdated());
        auto content = item->content();
        if (content.isEmpty()) {
            content = item->description();
        }

        if (itemId) {
            m_db.updateItemHeaders(*itemId, item->title(), date, authorName, item->link());
            if (!content.isEmpty()) {
                m_db.updateItemContent(*itemId, content);
            }

            // TODO this  sometimes creates an unnecessary item instance
            getById(*itemId); // push the update into any existing item instance

            op->setResult();
            return;
        }

        const auto &newId = m_db.insertItem(feedId, item->id(), item->title(), authorName, date, item->link(), content);
        if (!newId) {
            op->setResult();
            return;
        }
        ItemQuery result { m_db.selectItem(*newId) };
        appendArticleResults(op, result);
    });
}

void StorageImpl::onArticleReadChanged(ArticleImpl *article)
{
    const qint64 itemId { article->id() };
    const bool isRead { article->isRead() };
    m_db.updateItemRead(itemId, isRead);
}

void StorageImpl::onArticleStarredChanged(ArticleImpl *article)
{
    const qint64 itemId { article->id() };
    const bool isStarred { article->isStarred() };
    m_db.updateItemStarred(itemId, isStarred);
}

void StorageImpl::appendFeedResults(Future<Feed*> *op, FeedQuery &q)
{
    while (q.next()) {
        auto *ref = m_feedFactory.getInstance(q.id(), this);
        ref->updateFromQuery(q);
        op->appendResult(ref);
    }
}

Future<Feed*> *StorageImpl::getFeeds()
{
    return Future<Feed*>::yield(this, [this](auto *op){
        FeedQuery q { m_db.selectAllFeeds() };
        appendFeedResults(op, q);
    });
}

static qint64 packFeedUpdateInterval(Updater *updater)
{
    switch (updater->updateMode()) {
    case Updater::DefaultUpdateMode:
    default:
        return 0;

    case Updater::ManualUpdateMode:
        return -1;

    case Updater::CustomUpdateMode:
        return updater->updateInterval();
    }
}

Future<Feed*> *StorageImpl::storeFeed(Feed *feed)
{
    const QUrl &url = feed->url();
    const QString &name = feed->name();
    const qint64 updateInterval = packFeedUpdateInterval(feed->updater());
    return Future<Feed*>::yield(this, [this, url, name, updateInterval](auto *op){
        const auto &insertId = m_db.insertFeed(url);
        if (!insertId)
        {
            op->setResult();
            return;
        }
        m_db.updateFeedUpdateInterval(*insertId, updateInterval);
        m_db.updateFeedName(*insertId, name);
        FeedQuery result { m_db.selectFeed(*insertId) };
        appendFeedResults(op, result);
    });
}

static void onUpdateModeChanged(FeedDatabase &db, Updater *updater, qint64 feedId) {
    qint64 updateInterval = packFeedUpdateInterval(updater);
    db.updateFeedUpdateInterval(feedId, updateInterval);
}

static void onUpdateIntervalChanged(FeedDatabase &db, Updater *updater, qint64 feedId)
{
    if (updater->updateMode() != Updater::CustomUpdateMode) {
        return;
    }
    db.updateFeedUpdateInterval(feedId, updater->updateInterval());
}

void StorageImpl::listenForChanges(FeedImpl *feed)
{
    Updater *updater { feed->updater() };
    qint64 feedId = feed->id();
    QObject::connect(updater, &Updater::lastUpdateChanged, this, [this, updater, feedId]{
        m_db.updateFeedLastUpdate(feedId, updater->lastUpdate());
    });
    QObject::connect(updater, &Updater::updateIntervalChanged, this, [this, updater, feedId]{
        onUpdateIntervalChanged(m_db, updater, feedId);
    });
    QObject::connect(updater, &Updater::updateModeChanged, this, [this, updater, feedId]{
        onUpdateModeChanged(m_db, updater, feedId);
    });
    QObject::connect(updater, &Updater::expire, this, [this, feedId](auto olderThan){
        m_db.deleteItemsOlderThan(feedId, olderThan);
    });
    QObject::connect(feed, &Feed::nameChanged, this, [this, feed]{
        m_db.updateFeedName(feed->id(), feed->name());
    });
    QObject::connect(feed, &Feed::linkChanged, this, [this, feed]{
        m_db.updateFeedLink(feed->id(), feed->link().toString());
    });
    QObject::connect(feed, &Feed::iconChanged, this, [this, feed]{
        m_db.updateFeedIcon(feed->id(), feed->icon().toString());
    });
    QObject::connect(feed, &Feed::deleteRequested, this, [this, feed]{
        onFeedRequestDelete(feed);
    });
}
