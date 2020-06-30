#include "feedlistmodel.h"

FeedListModel::FeedListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_feeds.append({
        .id=-1,
        .headers={.name=""}
     });
}

int FeedListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_feeds.size();
}

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int indexRow = index.row();
    switch(role){
        case Roles::Id:
            return m_feeds[indexRow].id;

        case Roles::Name:
            return m_feeds[indexRow].headers.name;

        case Roles::UnreadCount:
            return 0;
    }

    return QVariant();
}

QHash<int, QByteArray> FeedListModel::roleNames() const
{
    return {
        {Roles::Id, "id"},
        {Roles::Name, "name"},
        {Roles::UnreadCount, "unreadCount"}
    };
}

static int findFeed(QVector<StoredFeed> const &list, qint64 feedId)
{
    for (int i=0; i<list.size(); i++)
    {
        if (list[i].id == feedId) return i;
    }
    return -1;
}

void FeedListModel::addFeed(StoredFeed const &feed)
{
    auto i = findFeed(m_feeds, feed.id);
    if (i<0) {
        beginInsertRows(QModelIndex(), m_feeds.size(), m_feeds.size());
        m_feeds.append(feed);
        endInsertRows();
    } else {
        m_feeds[i] = feed;
        auto idx = index(i);
        dataChanged(idx, idx);
    }
}
