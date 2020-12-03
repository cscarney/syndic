#include <QDebug>

#include "feeditemmodel.h"
#include "feedmanager.h"

FeedItemModel::FeedItemModel(QObject * parent):
    ItemModel(parent)
{

}

qint64 FeedItemModel::feedId()
{
    return m_feedId;
}

void FeedItemModel::setFeedId(qint64 feedId)
{
    if (m_feedId != feedId) {
        m_feedId = feedId;
        if (active()) refresh();
        emit feedIdChanged();
    }
}

void FeedItemModel::requestUpdate()
{
    manager()->requestUpdate(feedId());
}

ItemQuery *FeedItemModel::startQuery()
{
    return manager()->startQuery(m_feedId, unreadFilter());
}

bool FeedItemModel::itemFilter(const StoredItem &item)
{
    return item.feedId == m_feedId;
}

void FeedItemModel::setStatusFromUpstream()
{
    auto *manager = this->manager();
    if (!manager) {
        setStatus(LoadStatus::Idle);
        return;
    }
    setStatus( manager->getFeedStatus(m_feedId));
}
