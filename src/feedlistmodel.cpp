#include "feedlistmodel.h"

#include <QString>
#include <QVector>
#include <QIcon>

#include "context.h"
#include "feed.h"
#include "qmlfeedref.h"
#include "storeditem.h"
#include "feedstorageoperation.h"

using namespace FeedCore;

struct FeedListEntry {
    FeedRef feed;
    QString icon;
    int unreadCount;

    inline bool matchItem(const StoredItem &item) const
    {
        if (feed.get() != nullptr) {
            return feed == item.feedId;
        } else {
            return true;
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
        .icon="folder-symbolic",
        .unreadCount=0
    };
    feeds << item;
}

void FeedListModel::PrivData::addItem(const FeedRef &feed)
{
    FeedListEntry item = {
        .feed=feed,
        .icon="feed-subscribe",
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
            return QVariant::fromValue(QmlFeedRef(entry.feed));

        case Roles::Icon:
            return entry.icon;

        case Roles::UnreadCount:
            return entry.unreadCount;
    }

    return QVariant();
}


QHash<int, QByteArray> FeedListModel::roleNames() const
{
    return {
        {Roles::Ref, "feedRef"},
        {Roles::Icon, "icon"},
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

void FeedListModel::slotFeedAdded(const FeedRef &feed)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    priv->addItem(feed);
    endInsertRows();
}
