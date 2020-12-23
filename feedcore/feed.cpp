#include "feed.h"

namespace FeedCore{

Feed::Feed(QObject *parent):
    QObject(parent)
{

}

bool Feed::populateName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
        return true;
    }
    return false;
}

void Feed::populateUrl(const QUrl &url)
{
    setUrl(url);
}

void Feed::populateUnreadCount(int unreadCount)
{
    if (m_unreadCount != unreadCount) {
        int delta = unreadCount - m_unreadCount;
        m_unreadCount = unreadCount;
        emit unreadCountChanged(delta);
    }
}

void Feed::incrementUnreadCount(int delta)
{
    m_unreadCount += delta;
    emit unreadCountChanged(delta);
}

QString Feed::name() const
{
    return m_name;
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
