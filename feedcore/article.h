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
    Feed *feed() const;

    /**
     * Request the content of the article.
     *
     * The gotContent signal will be emitted with the requested content, possibly asyncronously.
     */
    Q_INVOKABLE virtual void requestContent() = 0;

    /**
     * Resolves a link relative to the article page
     */
    Q_INVOKABLE QUrl resolvedLink(const QUrl& link);

    const QString &title() const { return m_title; }
    const QString &author() const { return m_author; }
    const QDateTime &date() const { return m_date; }
    const QUrl &url() const { return m_url; }
    bool isRead() const { return m_readStatus; }
    virtual void setRead(bool isRead);
    bool isStarred() const { return m_starred; }
    virtual void setStarred(bool isStarred);

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
