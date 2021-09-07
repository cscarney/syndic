/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "starreditemsfeed.h"
#include "context.h"
using namespace FeedCore;

class StarredItemsFeed::StarredUpdater : public Updater
{
public:
    explicit StarredUpdater(StarredItemsFeed *parent)
        : Updater(parent, parent)
    {
    }

    void run() override
    {
        finish();
    }
};

StarredItemsFeed::StarredItemsFeed(FeedCore::Context *context, const QString &name, QObject *parent)
    : Feed(parent)
    , m_context{context}
    , m_updater{new StarredUpdater(this)}
{
    setName(name);
}

Future<ArticleRef> *StarredItemsFeed::getArticles(bool /*unused*/)
{
    return m_context->getStarred();
}

Feed::Updater *StarredItemsFeed::updater()
{
    return m_updater;
}
