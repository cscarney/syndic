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
        refresh();
        emit feedIdChanged();
    }
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
        setStatus(Ok);
        return;
    }
    switch (manager->getFeedStatus(m_feedId)) {
    case LoadStatus::Idle:
        setStatus(Ok);
        return;

    case LoadStatus::Updating:
        setStatus(Updating);
        return;

    case LoadStatus::Error:
        setStatus(Error);
        return;
    }
}