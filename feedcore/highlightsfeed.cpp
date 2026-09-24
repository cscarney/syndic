/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "highlightsfeed.h"
#include "context.h"

namespace FeedCore
{

HighlightsFeed::HighlightsFeed(Context *context, const QString &name, QObject *parent)
    : Feed(parent)
    , m_context{context}
{
    setName(name);
}

QFuture<ArticleRef> HighlightsFeed::getArticles(bool /*unused*/)
{
    return m_context->getHighlights();
}

}