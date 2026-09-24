/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "starreditemsfeed.h"
#include "context.h"
using namespace FeedCore;

StarredItemsFeed::StarredItemsFeed(FeedCore::Context *context, const QString &name, QObject *parent)
    : Feed(parent)
    , m_context{context}
{
    setName(name);
}

QFuture<ArticleRef> StarredItemsFeed::getArticles(bool /*unused*/)
{
    return m_context->getStarred();
}
