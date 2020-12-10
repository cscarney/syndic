#include "feedlistmodel.h"

#include <QString>
#include <QVector>
#include <QIcon>

#include "context.h"
#include "feed.h"
#include "storeditem.h"
#include "feedstorageoperation.h"
using namespace FeedCore;

struct FeedListEntry {
    FeedRef feed;
    FeedListModel::EntryType entryType;
    QString name;
    QString icon;
    LoadStatus status;
    int unreadCount;

    inline bool matchItem(const StoredItem &item) const
    {
        switch (entryType) {
        case FeedListModel::SingleFeedType:
            return feed == item.feedId;

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
    void addItem(const FeedRef &feed);
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
        .feed=FeedRef(),
        .entryType=AllType,
        .name=tr("All Items"),
        .icon="folder-symbolic",
        .status=LoadStatus::Idle,
        .unreadCount=0
    };
    feeds << item;
}

void FeedListModel::PrivData::addItem(const FeedRef &feed)
{
    auto *manager = parent->manager();
    FeedListEntry item = {
        .feed=feed,
        .entryType=SingleFeedType,
        .name=feed->name(),
        .icon="feed-subscribe",
        .status=manager->getFeedStatus(feed),
        .unreadCount=feed->unreadCount()
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
            const auto &index = parent->index(i);
            emit parent->dataChanged(index, index, {Roles::UnreadCount});
        }
    }
}

void FeedListModel::initialize()
{
    auto *q = manager()->startFeedQuery();
    QObject::connect(q, &FeedStorageOperation::finished, this, &FeedListModel::slotFeedQueryFinished);
    QObject::connect(manager(), &Context::itemAdded, this, &FeedListModel::slotItemAdded);
    QObject::connect(manager(), &Context::itemReadChanged, this, &FeedListModel::slotItemReadChanged);
    QObject::connect(manager(), &Context::feedStatusChanged, this, &FeedListModel::slotFeedStatusChanged);
    QObject::connect(manager(), &Context::feedNameChanged, this, &FeedListModel::slotFeedNameChanged);
    QObject::connect(manager(), &Context::feedAdded, this, &FeedListModel::slotFeedAdded);
}

int FeedListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return priv->feeds.size();
}

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int indexRow = index.row();
    const auto &entry = priv->feeds[indexRow];

    switch(role){
        case Roles::Ref:
            return QVariant::fromValue<FeedRefWrapper>(entry.feed);

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
        {Roles::Ref, "feedRef"},
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
    for (const auto &item : q->result()){
        priv->addItem(item);
        priv->feeds[0].unreadCount  += item->unreadCount();
    }
    endResetModel();
}

void FeedListModel::slotItemReadChanged(const StoredItem &item)
{
    int increment = item.status.isRead ? -1 : 1;
    priv->incrementUnreadCounts(item, increment);
}

void FeedListModel::slotItemAdded(const StoredItem &item)
{
    if (item.status.isRead) {
        return;
    }
    priv->incrementUnreadCounts(item, 1);
}

void FeedListModel::slotFeedStatusChanged(const FeedRef &feed, LoadStatus loadStatus)
{
    const int count = priv->feeds.count();
    for (int i = 0; i<count; i++) {
        auto &entry = priv->feeds[i];
        if ((entry.entryType == SingleFeedType) && (entry.feed == feed)) {
            entry.status = loadStatus;
            emit dataChanged(index(i), index(i));
        }
    }
}

void FeedListModel::slotFeedAdded(const FeedRef &feed)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    priv->addItem(feed);
    endInsertRows();
}

void FeedListModel::slotFeedNameChanged(const FeedRef &feed, const QString &newName)
{
    const int count = priv->feeds.count();
    for (int i = 0; i<count; i++) {
        auto &entry = priv->feeds[i];
        if ((entry.entryType == SingleFeedType) && (entry.feed == feed)) {
            entry.name = newName;
            emit dataChanged(index(i), index(i));
        }
    }
}
