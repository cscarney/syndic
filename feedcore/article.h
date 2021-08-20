/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FEEDCORE_ARTICLE_H
#define FEEDCORE_ARTICLE_H
#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <QPointer>

namespace FeedCore {
class Feed;

/**
 * Abstract class for stored articles.
 */
class Article : public QObject
{
    Q_OBJECT
    Q_PROPERTY(FeedCore::Feed *feed READ feed CONSTANT)
    Q_PROPERTY(bool isRead READ isRead WRITE setRead NOTIFY readStatusChanged);
    Q_PROPERTY(bool isStarred READ isStarred WRITE setStarred NOTIFY starredChanged);
    Q_PROPERTY(QString title READ title NOTIFY titleChanged);
    Q_PROPERTY(QString author READ author NOTIFY authorChanged);
    Q_PROPERTY(QDateTime date READ date NOTIFY dateChanged);
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged);
public:
    /**
     * The feed that this article belongs to
     */
    Feed *feed() const;

    /**
     * The title/headline of the article
     */
    const QString &title() const { return m_title; }

    /**
     * The name of the author.
     */
    const QString &author() const { return m_author; }

    /**
     * The date the article was last updated.
     */
    const QDateTime &date() const { return m_date; }

    /**
     * Link to a web page containing the full article.
     */
    const QUrl &url() const { return m_url; }

    /**
     * True if the article has been read.
     */
    bool isRead() const { return m_readStatus; }

    /**
     * Set whether the article has been read.
     *
     * Derived classes that override this method must call the base class implementation.
     */
    virtual void setRead(bool isRead);

    /**
     * True if the article has been marked as starred.
     */
    bool isStarred() const { return m_starred; }

    /**
     * Set whether the article has been marked as starred.
     *
     * Derived classes that override this method must call the base class implementation.
     */
    virtual void setStarred(bool isStarred);

    /**
     * Request the content of the article.
     *
     * The gotContent signal will be emitted with the requested content, possibly asyncronously.
     */
    Q_INVOKABLE virtual void requestContent() = 0;

    /**
     * Resolves a link relative to the article page
     */
    Q_INVOKABLE QUrl resolvedLink(QUrl link);
signals:
    void titleChanged();
    void authorChanged();
    void dateChanged();
    void urlChanged();
    void readStatusChanged();
    void starredChanged();
    void gotContent(const QString &content);
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
    bool m_readStatus { false };
    bool m_starred { false };
};
}

#endif // FEEDCORE_ARTICLE_H
