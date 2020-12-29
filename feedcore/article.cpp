#include "article.h"

namespace FeedCore {

Article::Article(const FeedRef &feed, QObject *parent) :
    QObject(parent),
    m_feed(feed)
{

}

void Article::populateTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

void Article::populateAuthor(const QString &author)
{
    if (m_author != author) {
        m_author = author;
        emit authorChanged();
    }
}

void Article::populateDate(const QDateTime &date)
{
    if (m_date != date) {
        m_date = date;
        emit dateChanged();
    }
}

void Article::populateUrl(const QUrl &url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
    }
}

void Article::populateReadStatus(bool isRead)
{
    if (m_readStatus != isRead) {
        m_readStatus = isRead;
        emit readStatusChanged();
    }
}

FeedCore::FeedRef FeedCore::Article::feed()
{
    return m_feed;
}

QString Article::title()
{
    return m_title;
}

QString Article::author()
{
    return m_author;
}

QDateTime Article::date()
{
    return m_date;
}

QUrl Article::url()
{
    return m_url;
}

bool Article::isRead() const
{
    return m_readStatus;
}

}
