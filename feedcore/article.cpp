#include "article.h"
using namespace FeedCore;

Article::Article(QObject *parent) :
    QObject(parent)
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

const QString &Article::title() const
{
    return m_title;
}

const QString &Article::author() const
{
    return m_author;
}

const QDateTime &Article::date() const
{
    return m_date;
}

const QUrl &Article::url() const
{
    return m_url;
}

bool Article::isRead() const
{
    return m_readStatus;
}
