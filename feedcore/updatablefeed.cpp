/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "updatablefeed.h"
#include "context.h"
#include "updaterfactory.h"
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>
#include <QQueue>
#include <Syndication/Image>
#include <Syndication/ParserCollection>
using namespace FeedCore;

Feed::Updater *UpdatableFeed::updater()
{
    if (!m_updater) {
        auto *ctx = context();
        auto *factory = ctx ? ctx->updaterFactory() : UpdaterFactory::instance();
        m_updater = factory->createUpdater(this);
        m_updater->setParent(this);
    }
    return m_updater;
}

FeedCore::UpdatableFeed::UpdatableFeed(QObject *parent)
    : Feed(parent)
    , m_updater{nullptr}
{
}

static inline bool hasImageUrl(const Syndication::ImagePtr &image)
{
    return !(image.isNull() || image->url().isEmpty());
}

static inline QUrl getIconUrl(const Syndication::FeedPtr &feed, const QUrl &feedUrl)
{
    const Syndication::ImagePtr icon = feed->icon();
    if (hasImageUrl(icon)) {
        return feedUrl.resolved(icon->url());
    }

    const Syndication::ImagePtr image = feed->image();
    if (hasImageUrl(image)) {
        return feedUrl.resolved(image->url());
    }

    return QUrl();
}

void UpdatableFeed::updateFromSource(const Syndication::FeedPtr &feed)
{
    if (name().isEmpty()) {
        setName(feed->title());
    }
    setLink(feed->link());
    setIcon(getIconUrl(feed, url()));
    const auto &items = feed->items();
    time_t expireTime = 0;
    if ((expireAge() > 0) && (expireMode() != DisableUpdateMode)) {
        expireTime = updater()->updateStartTime().toSecsSinceEpoch() - expireAge();
    }
    for (const auto &item : items) {
        const auto &dateUpdated = item->dateUpdated();
        if (dateUpdated == 0 || dateUpdated >= expireTime) {
            updateSourceArticle(item);
        }
    }
    if (expireTime > 0) {
        expire(QDateTime::fromSecsSinceEpoch(expireTime));
    }
}
