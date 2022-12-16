/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ttrss/storageimpl.h"
#include "ttrss/articleimpl.h"
#include "ttrss/feedimpl.h"
#include <QJsonArray>
#include <QJsonObject>
using namespace FeedCore;

FeedCore::Future<FeedCore::ArticleRef> *TTRSS::StorageImpl::getAll()
{
    return getArticles(Client::SPECIAL_FEED_ALL, "all_articles", 0);
}

FeedCore::Future<FeedCore::ArticleRef> *TTRSS::StorageImpl::getUnread()
{
    return getArticles(Client::SPECIAL_FEED_ALL, "unread", 0);
}

FeedCore::Future<FeedCore::ArticleRef> *TTRSS::StorageImpl::getStarred()
{
    return getArticles(Client::SPECIAL_FEED_STARRED, "all_articles", 0);
}

FeedCore::Future<FeedCore::Feed *> *TTRSS::StorageImpl::getFeeds()
{
    auto *result = new Future<FeedCore::Feed *>;
    Client::ApiCall *apiCall = m_client.getFeeds();
    apiCall->onError(this, [result] {
        result->finish();
    });
    apiCall->onSuccess(this, [this, result](const QJsonDocument &body) {
        const QJsonArray feeds = body.object()["content"].toArray();
        for (const QJsonValue &feedJson : feeds) {
            QJsonObject jFeed = feedJson.toObject();
            int id = jFeed["id"].toInt();
            FeedImpl *feed = m_feedFactory.getInstance(id, this);
            feed->updateFromJson(jFeed);
            result->appendResult(feed);
        }
        result->finish();
    });
    return result;
}

FeedCore::Future<FeedCore::Feed *> *TTRSS::StorageImpl::storeFeed(FeedCore::Feed * /*feed*/)
{
    return Future<Feed *>::yield(this, [](auto) {});
}

FeedCore::Future<ArticleRef> *TTRSS::StorageImpl::getArticles(int feedId, const QString &mode, int sinceId)
{
    auto *result = new Future<FeedCore::ArticleRef>;
    Client::ApiCall *apiCall = m_client.getArticles(feedId, mode, sinceId);
    apiCall->onError(this, [result] {
        result->finish();
    });
    apiCall->onSuccess(this, [this, result](const QJsonDocument &body) {
        const QJsonArray articles = body.object()["content"].toArray();
        for (const QJsonValue &articleJson : articles) {
            QJsonObject jArticle = articleJson.toObject();
            int id = jArticle["id"].toInt();
            int feedId = jArticle["feed_id"].toInt();
            FeedImpl *feed = m_feedFactory.getInstance(feedId, this);
            QSharedPointer<ArticleImpl> article = m_articleFactory.getInstance(id, feed, this);
            article->updateFromJson(jArticle);
            result->appendResult(article);
        }
        result->finish();
    });
    return result;
}
