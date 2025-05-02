/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "highlightsfeed.h"
#include "context.h"

namespace FeedCore
{

class HighlightsFeed::HighlightsUpdater : public Updater
{
public:
    explicit HighlightsUpdater(HighlightsFeed *parent)
        : Updater(parent, parent)
    {
    }

    void run() override
    {
        finish();
    }
};

HighlightsFeed::HighlightsFeed(Context *context, const QString &name, QObject *parent)
    : Feed(parent)
    , m_context{context}
    , m_updater{new HighlightsUpdater(this)}
{
    setName(name);
}

QFuture<ArticleRef> HighlightsFeed::getArticles(bool /*unused*/)
{
    return m_context->getHighlights();
}

Feed::Updater *HighlightsFeed::updater()
{
    return m_updater;
}

}