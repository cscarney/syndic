/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "article.h"
#include "feed.h"
using namespace FeedCore;

Article::Article(Feed *feed, QObject *parent) :
    QObject(parent),
    m_feed(feed)
{
}

void Article::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

void Article::setAuthor(const QString &author)
{
    if (m_author != author) {
        m_author = author;
        emit authorChanged();
    }
}

void Article::setDate(const QDateTime &date)
{
    if (m_date != date) {
        m_date = date;
        emit dateChanged();
    }
}

void Article::setUrl(const QUrl &url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
    }
}

Feed *Article::feed() const
{
    return m_feed;
}

void Article::setRead(bool isRead)
{
    if (m_readStatus != isRead) {
        m_readStatus = isRead;
        emit readStatusChanged();
    }
}

void Article::setStarred(bool isStarred)
{
    if (m_starred != isStarred) {
        m_starred = isStarred;
        emit starredChanged();
    }
}

QUrl Article::resolvedLink(QUrl link)
{
    return m_url.resolved(link);
}
