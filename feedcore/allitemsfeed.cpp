/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "allitemsfeed.h"
#include "context.h"
using namespace FeedCore;

AllItemsFeed::AllItemsFeed(Context *context, const QString &name, QObject *parent)
    : AggregateFeed(parent)
    , m_context{context}
{
    setName(name);
    for (const auto &feed : context->getFeeds()) {
        addFeed(feed);
    }
    QObject::connect(context, &Context::feedAdded, this, &AllItemsFeed::addFeed);
}

Future<ArticleRef> *AllItemsFeed::getArticles(bool unreadFilter)
{
    return m_context->getArticles(unreadFilter);
}
