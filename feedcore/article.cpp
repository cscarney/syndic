/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "article.h"
#include "context.h"
#include "feed.h"
#include "readability/readability.h"
#include "readability/readabilityresult.h"
#include <QLocale>

using namespace FeedCore;

Article::Article(Feed *feed, QObject *parent)
    : QObject(parent)
    , m_feed(feed)
    , m_author(feed->name())
{
}

void Article::setTitle(const QString &title)
{
    if (title.isEmpty() && m_date.isValid()) {
        QLocale l;
        m_title = l.toString(m_date.date());
        emit titleChanged();
    } else if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

void Article::setAuthor(const QString &author)
{
    if (m_author != author) {
        if (author.isEmpty()) {
            m_author = m_feed->name();
        } else {
            m_author = author;
        }
        emit authorChanged();
    }
}

void Article::setDate(const QDateTime &date)
{
    if (m_date != date) {
        m_date = date;
        emit dateChanged();

        if (m_title.isEmpty()) {
            setTitle(QString());
        }
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

void Article::requestReadableContent(Readability *readability)
{
    if (readability == nullptr) {
        requestContent();
        return;
    }

    ReadabilityResult *result = readability->fetch(url());
    QObject::connect(result, &ReadabilityResult::finished, this, [this](auto content) {
        emit gotContent(content, ReadableContent);
        cacheReadableContent(content);
    });
    QObject::connect(result, &ReadabilityResult::error, this, [this]() {
        requestContent();
    });
}

void Article::requestReadableContent(Context *context)
{
    requestReadableContent(context->getReadability());
}

void Article::cacheReadableContent(const QString & /* readableContent */)
{
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

QUrl Article::resolvedLink(const QUrl &link)
{
    return m_url.resolved(link);
}
