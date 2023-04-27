/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "factory.h"
#include "future.h"
#include "updatablefeed.h"
#include <Syndication/Feed>
namespace FeedCore
{
/**
 * A minimal implementation of Feed, used for configuring and previewing feeds before commiting them to storage.
 *
 * This implementation provides preview content backed directly by a Syndication::Feed instance.
 * Preview content is not downloaded automatically; call updater()->start() to fetch it from the remote source.
 *
 * The preview implementation does not do any state tracking -- marking a preview article as read, etc. is a no-op
 * and every update obliterates all previous content.
 */
class ProvisionalFeed : public UpdatableFeed
{
    Q_OBJECT

    /**
     * The feed being edited.
     *
     * To use ProvisionalFeed to edit an existing feed, set targetFeed to the existing feed object.
     * The properties of the target feed are copied into the provisional feed, which can then be
     * edited while leaving the original feed intact.  To copy the changed properties back to
     * the original feed object, call ProvisionalFeed::save().
     *
     * This property is null by default.
     */
    Q_PROPERTY(Feed *targetFeed READ targetFeed WRITE setTargetFeed NOTIFY targetFeedChanged)

    /**
     * User input for the URL string.
     *
     * Setting this property will attempt to parse a URL from the string input. If urlString
     * cannot be converted into a valid URL, the URL is not changed.
     */
    Q_PROPERTY(QString urlString READ urlString WRITE setUrlString NOTIFY urlStringChanged)

public:
    explicit ProvisionalFeed(QObject *parent = nullptr);

    enum UrlStringStatus { INVALID, VALID, PENDING };
    Q_ENUM(UrlStringStatus)

    /**
     * Copy this feed's configuration into the feed specified by the targetFeed property
     */
    Q_INVOKABLE void save();

    /**
     * Reset URL string to the current feed url
     */
    Q_INVOKABLE void syncUrlString();

    /**
     * Whether urlString was successfully parsed into a URL.
     */
    Q_INVOKABLE ProvisionalFeed::UrlStringStatus urlStringStatus()
    {
        return m_urlStringStatus;
    }

    QFuture<ArticleRef> getArticles(bool unreadFilter) final;

    Feed *targetFeed() const;
    void setTargetFeed(Feed *targetFeed);

    const QString &urlString() const;
    void setUrlString(const QString &newUrlString);

signals:
    void targetFeedChanged();
    void urlStringChanged();
    void saveFailed();

private:
    Feed *m_targetFeed{nullptr};
    Syndication::FeedPtr m_feed;
    QString m_urlString;
    UrlStringStatus m_urlStringStatus{INVALID};

    class ArticleImpl;
    SharedFactory<Syndication::ItemPtr, ArticleImpl> m_articles;
    void onUrlChanged();
    void updateFromSource(const Syndication::FeedPtr &feed) final;
    void updateSourceArticle(const Syndication::ItemPtr & /*article*/) final{};
    void expire(const QDateTime & /*olderThan*/) final{};
};
}
