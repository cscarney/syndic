#include "cmake-config.h"
#include "newitemnotifier.h"
#include "article.h"
#include "context.h"
#include "allitemsfeed.h"

#ifdef KF5Notifications_FOUND
#include "knotification.h"
#endif

#include <QDebug>
using namespace FeedCore;

NewItemNotifier::NewItemNotifier(QObject *parent) : QObject(parent)
{

}

void NewItemNotifier::setContext(FeedCore::Context *context)
{
    if (m_context != context) {
        if (m_feed) {
            QObject::disconnect(m_feed, nullptr, this, nullptr);
        }
        m_feed = new AllItemsFeed(context, "", this);
        m_counter = 0;
        QObject::connect(m_feed, &Feed::statusChanged, this, &NewItemNotifier::onStatusChanged);
        QObject::connect(m_feed, &Feed::articleAdded, this, &NewItemNotifier::onArticleAdded);
        emit contextChanged();
    }
}

FeedCore::Context *NewItemNotifier::context() const
{
    return m_context;
}

void NewItemNotifier::setEnabled(bool flag)
{
    if (m_enabled != flag) {
        m_enabled = flag;
        emit enabledChanged();
    }
}

bool NewItemNotifier::enabled() const
{
    return m_enabled;
}

void NewItemNotifier::onStatusChanged()
{
    if (m_enabled && (m_counter > 0)) {
        postNotification();
    }
    m_counter = 0;
}

void NewItemNotifier::onArticleAdded(const ArticleRef &article){
    if ((m_feed->status() == LoadStatus::Updating) && (!article->isRead())) {
        ++m_counter;
    }
}

void NewItemNotifier::postNotification()
{
    QString notificationText { tr("%1 New Item(s)", "Notification Text", m_counter).arg(m_counter) };
    qDebug() << "Notification: " << notificationText;
#ifdef KF5Notifications_FOUND
    KNotification::event(KNotification::Notification, notificationText); 
#endif
}
