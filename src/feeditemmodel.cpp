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
            qDebug() << "set feed after initialization!!";
        }
        emit feedChanged();
    }
}

void FeedItemModel::initialize() {
    auto *m = manager();
    QObject::connect(m_feed.get(), &Feed::itemAdded, this, &FeedItemModel::slotItemAdded);
    QObject::connect(m, &Context::itemChanged, this, &FeedItemModel::slotItemChanged);
    QObject::connect(m_feed.get(), &Feed::statusChanged, this, &FeedItemModel::slotStatusChanged);
    refresh();
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

void FeedItemModel::setStatusFromUpstream()
{
    auto *manager = this->manager();
    if (manager == nullptr) {
        setStatus(LoadStatus::Idle);
        return;
    }
    setStatus( manager->getFeedStatus(m_feed));
}

void FeedItemModel::slotStatusChanged()
{
    setStatus(m_feed->status());
}
