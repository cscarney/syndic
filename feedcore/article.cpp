#include "article.h"
using namespace FeedCore;

Article::Article(QObject *parent) :
    QObject(parent)
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

void Article::setRead(bool isRead)
{
    if (m_readStatus != isRead) {
        m_readStatus = isRead;
        emit readStatusChanged();
    }
}
