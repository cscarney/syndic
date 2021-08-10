#include "feed.h"
#include "updater.h"

namespace FeedCore{

Feed::Feed(QObject *parent):
    QObject(parent)
{

}

Feed::~Feed()
{
    setStatus(Idle);
    setUnreadCount(0);
}

void Feed::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void Feed::setUnreadCount(int unreadCount)
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

void Feed::setUrl(const QUrl &url)
{
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
    }
}

void Feed::setLink(const QUrl &link)
{
    if (m_link != link) {
        m_link = link;
        emit linkChanged();
    }
}

void Feed::setIcon(const QUrl &icon)
{
    if (!icon.isValid()){return;}
    if (m_icon != icon){
        m_icon = icon;
        emit iconChanged();
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

void Feed::updateParams(Feed *other)
{
    if (other==nullptr) {return;}
    setName(other->name());
    setUrl(other->url());
    this->updater()->updateParams(other->updater());
}

}
