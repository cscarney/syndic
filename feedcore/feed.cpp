#include "feed.h"

namespace FeedCore{

Feed::Feed(QObject *parent):
    QObject(parent)
{

}

void Feed::populateName(const QString &name)
{
    setName(name);
}

void Feed::populateUrl(const QUrl &url)
{
    setUrl(url);
}

void Feed::populateUnreadCount(int unreadCount)
{
    if (m_unreadCount != unreadCount) {
        m_unreadCount = unreadCount;
        emit unreadCountChanged();
    }
}

QString Feed::name() const
{
    return m_name;
}

void Feed::setName(const QString &s)
{
    if (m_name != s) {
        m_name = s;
        emit nameChanged();
    }
}

QUrl Feed::url() const
{
    return m_url;
}

void Feed::setUrl(const QUrl &url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
    }
}

int Feed::unreadCount() const
{
    return m_unreadCount;
}

LoadStatus Feed::status() const
{
    return m_status;
}

void Feed::setStatus(LoadStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

}
