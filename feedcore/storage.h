/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_STORAGE_H
#define FEEDCORE_STORAGE_H
#include "future.h"
#include <QObject>
#include <Syndication/Feed>
#include <Syndication/Item>

namespace FeedCore
{
class Feed;

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent = nullptr)
        : QObject(parent){};
    virtual Future<ArticleRef> *getAll() = 0;
    virtual Future<ArticleRef> *getUnread() = 0;
    virtual Future<ArticleRef> *getStarred() = 0;
    virtual Future<Feed *> *getFeeds() = 0;
    virtual Future<Feed *> *storeFeed(Feed *feed) = 0;
};
}
#endif // FEEDCORE_STORAGE_H
