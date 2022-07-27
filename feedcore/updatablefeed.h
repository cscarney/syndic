/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UPDATABLEFEED_H
#define UPDATABLEFEED_H
#include "feed.h"
#include <Syndication/Feed>
#include <Syndication/Item>
namespace FeedCore
{
/**
 * Base class for feed implementations that are updated locally using the Syndication library
 */
class UpdatableFeed : public Feed
{
public:
    Updater *updater() final;

protected:
    explicit UpdatableFeed(QObject *parent);

private:
    /**
     * Process an update from the remote source.
     *
     * This is called whenever an update has been sucessfully downloaded and processed
     * by the Syndication library.  The base implementation updates the properties of the
     * feed using the retrieved data, then calls updateSourceArticle for each article.
     */
    virtual void updateFromSource(const Syndication::FeedPtr &feed);

    /**
     * Process an article from the remote source.
     *
     * This is called by the base implmentation of updateFromSource.  Derived classes
     * should implement this to create insances of their corresponding article implementation.
     * The implementation is responsible for identifying duplicates, and should emit the
     * Feed::articleAdded signal when a new article is found.
     */
    virtual void updateSourceArticle(const Syndication::ItemPtr &article) = 0;

    /**
     * Delete old articles
     *
     * This is called by the base implementation of updateFromSource.  Derivce classes
     * should override this and remove old articles from storage.
     */
    virtual void expire(const QDateTime &olderThan) = 0;

    class UpdaterImpl;
    UpdaterImpl *m_updater;
};

}

#endif // UPDATABLEFEED_H
