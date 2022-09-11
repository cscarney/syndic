/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "article.h"
#include "context.h"
#include "feed.h"
#include "readability/readability.h"

using namespace FeedCore;

Article::Article(Feed *feed, QObject *parent)
    : QObject(parent)
    , m_feed(feed)
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

void Article::requestReadableContent(Readability *readability)
{
    if (readability == nullptr) {
        requestContent();
        return;
    }

    struct Connections {
        QMetaObject::Connection success;
        QMetaObject::Connection error;
        void disconnect() const
        {
            QObject::disconnect(success);
            QObject::disconnect(error);
        }
    };
    auto connections = std::make_shared<Connections>();
    QString urlString = url().toString();

    connections->success = QObject::connect(readability, &Readability::finishedFetching, this, [this, connections, urlString](auto gotUrl, auto content) {
        if (gotUrl == urlString) {
            emit gotContent(content, ReadableContent);
            cacheReadableContent(content);
            connections->disconnect();
        }
    });

    connections->error = QObject::connect(readability, &Readability::errorFetching, this, [this, connections, urlString](auto errorUrl) {
        if (errorUrl == urlString) {
            requestContent();
            connections->disconnect();
        }
    });

    readability->fetch(urlString);
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
