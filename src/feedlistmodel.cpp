#include <QString>
#include <QVector>
#include <QIcon>
#include <QDebug>

#include "feedlistmodel.h"

struct FeedListEntry {
    qint64 id;
    FeedListModel::EntryType entryType;
    QString name;
    QString icon;
    LoadStatus status;
    int unreadCount;

    inline bool matchItem(const StoredItem item)
    {
        switch (entryType) {
        case FeedListModel::SingleFeedType:
            return id == item.feedId;

        case FeedListModel::AllType:
            return true;

        case FeedListModel::StarredType:
            return item.status.isStarred;

        default:
            return false;
        }
    }
};

class FeedListModel::PrivData {
public:
    FeedListModel *parent;
    QVector<FeedListEntry> feeds;

    PrivData(FeedListModel *parent) :
        parent(parent)
    {}

    void addAllFeedsItem();
    void addItem(const StoredFeed &feed);
    void incrementUnreadCounts(const StoredItem &matchItem, int increment);
};

FeedListModel::FeedListModel(QObject *parent)
    : ManagedListModel(parent),
      priv(std::make_unique<PrivData>(this))
{

}

FeedListModel::~FeedListModel()=default;

void FeedListModel::PrivData::addAllFeedsItem()
{
    FeedListEntry item = {
        .id=-1,
        .entryType=AllType,
        .name=tr("All Items"),
        .icon="folder-symbolic",
        .status=LoadStatus::Idle,
        .unreadCount=0
    };
    feeds << item;
}

void FeedListModel::PrivData::addItem(const StoredFeed &feed)
{
    auto *manager = parent->manager();
    FeedListEntry item = {
        .id=feed.id,
        .entryType=SingleFeedType,
        .name=feed.headers.name,
        .icon="feed-subscribe",
        .status=manager->getFeedStatus(feed.id),
        .unreadCount=feed.unreadCount
    };
    feeds << item;
}

void FeedListModel::PrivData::incrementUnreadCounts(const StoredItem &matchItem, int increment)
{
    auto size = feeds.size();
    for (int i=0; i<size; i++) {
        auto &entry = feeds[i];
        if (entry.matchItem(matchItem)){
            entry.unreadCount += increment;
            auto index = parent->index(i);
            parent->dataChanged(index, index, {Roles::UnreadCount});
        }
    }
}

void FeedListModel::initialize()
{
    auto *q = manager()->startFeedQuery();
    QObject::connect(q, &FeedStorageOperation::finished, this, &FeedListModel::slotFeedQueryFinished);
    QObject::connect(manager(), &FeedManager::itemReadChanged, this, &FeedListModel::slotItemReadChanged);
    QObject::connect(manager(), &FeedManager::feedStatusChanged, this, &FeedListModel::slotFeedStatusChanged);
}

int FeedListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return priv->feeds.size();
}

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int indexRow = index.row();
    const auto &entry = priv->feeds[indexRow];

    switch(role){
        case Roles::Id:
            return entry.id;

        case Roles::Type:
            return entry.entryType;

        case Roles::Name:
            return entry.name;

        case Roles::Icon:
            return entry.icon;

        case Roles::Status:
            return entry.status;

        case Roles::UnreadCount:
            return entry.unreadCount;
    }

    return QVariant();
}


QHash<int, QByteArray> FeedListModel::roleNames() const
{
    return {
        {Roles::Id, "id"},
        {Roles::Type, "entryType"},
        {Roles::Name, "name"},
        {Roles::Icon, "icon"},
        {Roles::Status, "status"},
        {Roles::UnreadCount, "unreadCount"}
    };
}

void FeedListModel::slotFeedQueryFinished()
{
    beginResetModel();
    auto *q = static_cast<FeedQuery *>(QObject::sender());
    priv->feeds.clear();
    priv->addAllFeedsItem();
    for (const auto &item : q->result){
        priv->addItem(item);
        priv->feeds[0].unreadCount  += item.unreadCount;
    }
    endResetModel();
}

void FeedListModel::slotItemReadChanged(const StoredItem &item)
{
    auto increment = item.status.isRead ? -1 : 1;
    priv->incrementUnreadCounts(item, increment);
}

void FeedListModel::slotItemAdded(const StoredItem &item)
{
    if (item.status.isRead) return;
    priv->incrementUnreadCounts(item, 1);
}

void FeedListModel::slotFeedStatusChanged(qint64 feedId, LoadStatus loadStatus)
{
    auto count = priv->feeds.count();
    for (int i = 0; i<count; i++) {
        auto &entry = priv->feeds[i];
        if ((entry.entryType == SingleFeedType) && (entry.id == feedId)) {
            entry.status = loadStatus;
            dataChanged(index(i), index(i));
        }
    }
}
