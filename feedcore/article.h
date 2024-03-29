/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "articleref.h"
#include "future.h"
#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QUrl>

namespace FeedCore
{
class Feed;
class Context;
class Readability;

/**
 * Abstract class for stored articles.
 */
class Article : public QObject
{
    Q_OBJECT

    /**
     * The feed that this article belongs to
     */
    Q_PROPERTY(FeedCore::Feed *feed READ feed CONSTANT)

    /**
     * True if the article has been read.
     */
    Q_PROPERTY(bool isRead READ isRead WRITE setRead NOTIFY readStatusChanged);

    /**
     * True if the article has been marked as starred.
     */
    Q_PROPERTY(bool isStarred READ isStarred WRITE setStarred NOTIFY starredChanged);

    /**
     * The title/headline of the article
     */
    Q_PROPERTY(QString title READ title NOTIFY titleChanged);

    /**
     * The name of the author.  If there are multiple authors, this is the first one.
     */
    Q_PROPERTY(QString author READ author NOTIFY authorChanged);

    /**
     * The date the article was last updated, according to the remote feed source.
     */
    Q_PROPERTY(QDateTime date READ date NOTIFY dateChanged);

    /**
     * Link to a web page containing the full article.
     */
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged);

public:
    enum ContentType { FeedContent, ReadableContent };
    Q_ENUM(ContentType);

    Feed *feed() const;

    /**
     * Request the content of the article.
     *
     * The gotContent signal will be emitted with the requested content, possibly asyncronously.
     */
    Q_INVOKABLE virtual void requestContent() = 0;

    /**
     * Requests the content of the article's linked web page.
     *
     * The article url will be loaded from the web and processed by readability tool if it is
     * available.  The gotContent signal will be emitted with the readable content, possibly
     * asyncronously.
     */
    void requestReadableContent(FeedCore::Readability *readability, bool forceReload = false);

    /**
     * Requestes the content of the article's linked web page.
     *
     * This overload uses the readability implementation provided by \a context
     */
    Q_INVOKABLE void requestReadableContent(FeedCore::Context *context, bool forceReload = false);

    /**
     * Called by requestReadableContent when readability successfully extracts web content.
     *
     * The base implementation does nothing.
     */
    virtual void cacheReadableContent(const QString &readableContent);

    /**
     * Called by requestReadableContent before trying to extract web content.
     *
     * Derived classes that implement cacheReadableContent should override this
     * to return the content from the cache. If there is no cached content,
     * the implementation may return either nullptr, or yield an empty result set.
     *
     * The default implementation returns nullptr.
     */
    virtual QFuture<QString> getCachedReadableContent();

    /**
     * Resolves a link relative to the article page
     */
    Q_INVOKABLE QUrl resolvedLink(const QUrl &link);

    const QString &title() const
    {
        return m_title;
    }
    const QString &author() const
    {
        return m_author;
    }
    const QDateTime &date() const
    {
        return m_date;
    }
    const QUrl &url() const
    {
        return m_url;
    }
    bool isRead() const
    {
        return m_readStatus;
    }
    virtual void setRead(bool isRead);
    bool isStarred() const
    {
        return m_starred;
    }
    virtual void setStarred(bool isStarred);

signals:
    void titleChanged();
    void authorChanged();
    void dateChanged();
    void urlChanged();
    void readStatusChanged();
    void starredChanged();
    void gotContent(const QString &content, FeedCore::Article::ContentType type = FeedContent);

protected:
    explicit Article(Feed *feed, QObject *parent = nullptr);
    void setTitle(const QString &title);
    void setAuthor(const QString &author);
    void setDate(const QDateTime &date);
    void setUrl(const QUrl &url);

private:
    QPointer<Feed> m_feed;
    QString m_title;
    QString m_author;
    QDateTime m_date;
    QUrl m_url;
    bool m_readStatus{false};
    bool m_starred{false};

    void setDefaultTitle();
    void reloadReadableContent(FeedCore::Readability *readability);
};
}

Q_DECLARE_METATYPE(FeedCore::ArticleRef)
