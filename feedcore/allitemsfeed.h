/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "aggregatefeed.h"
#include <QSet>

namespace FeedCore
{
class Context;

/**
 * A Feed implementation that combines all articles from a context.
 */
class AllItemsFeed : public AggregateFeed
{
    Q_OBJECT
public:
    AllItemsFeed(Context *context, const QString &name, QObject *parent = nullptr);
    Future<ArticleRef> *getArticles(bool unreadFilter) final;

private:
    Context *m_context{nullptr};
};
}
