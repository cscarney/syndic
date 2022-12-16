/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "factory.h"
#include "storage.h"
#include "ttrss/ttrssclient.h"

namespace TTRSS
{
class ArticleImpl;
class FeedImpl;
class StorageImpl : public FeedCore::Storage
{
    Q_OBJECT
public:
    FeedCore::Future<FeedCore::ArticleRef> *getAll() override;
    FeedCore::Future<FeedCore::ArticleRef> *getUnread() override;
    FeedCore::Future<FeedCore::ArticleRef> *getStarred() override;
    FeedCore::Future<FeedCore::Feed *> *getFeeds() override;
    FeedCore::Future<FeedCore::Feed *> *storeFeed(FeedCore::Feed *feed) override;
    FeedCore::Future<FeedCore::ArticleRef> *getArticles(int feedId, const QString &mode, int sinceId);
    Client &client()
    {
        return m_client;
    }

private:
    Client m_client;
    FeedCore::SharedFactory<int, ArticleImpl> m_articleFactory;
    FeedCore::ObjectFactory<int, FeedImpl> m_feedFactory;
};

}
