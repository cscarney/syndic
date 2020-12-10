#include "feeditemmodel.h"

#include <QDebug>

#include "context.h"
#include "storeditem.h"
using namespace FeedCore;

FeedItemModel::FeedItemModel(QObject * parent):
    ItemModel(parent)
{

}

FeedRef FeedItemModel::feed() const
{
    return m_feed;
}

void FeedItemModel::setFeed(const FeedRef &feed)
{
    if (m_feed != feed) {
        m_feed = feed;
        if (active()) {
            refresh();
        }
        emit feedChanged();
    }
}

FeedRefWrapper FeedItemModel::feedWrapper() const
{
    return feed();
}

void FeedItemModel::setFeedWrapper(const FeedRefWrapper &feed)
{
    setFeed(feed);
}

void FeedItemModel::requestUpdate()
{
    manager()->requestUpdate(feed());
}

ItemQuery *FeedItemModel::startQuery()
{
    if (m_feed.isNull()) {
        qDebug() << "starting query with no feed set!";
        return nullptr;
    }
    return manager()->startQuery(m_feed, unreadFilter());
}

bool FeedItemModel::itemFilter(const StoredItem &item)
{
    return item.feedId == m_feed;
}

void FeedItemModel::setStatusFromUpstream()
{
    auto *manager = this->manager();
    if (manager == nullptr) {
        setStatus(LoadStatus::Idle);
        return;
    }
    setStatus( manager->getFeedStatus(m_feed));
}
