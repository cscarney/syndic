/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "feed.h"

namespace FeedCore
{
class Context;

/**
 * A Feed implementation which displays highlights from
 * all feeds in a context.
 */
class HighlightsFeed : public Feed
{
public:
    HighlightsFeed(Context *context, const QString &name, QObject *parent = nullptr);
    QFuture<ArticleRef> getArticles(bool unreadFilter) final;
    Updater *updater() final;

private:
    Context *m_context{nullptr};
    Updater *m_updater{nullptr};
    class HighlightsUpdater;
};
}