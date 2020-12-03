#include <QSortFilterProxyModel>
#include <QDebug>
#include <QList>

#include "itemmodel.h"
#include "feedmanager.h"
#include "feedstorageoperation.h"

struct ItemModel::PrivData {
    QList<StoredItem> items;
    bool unreadFilter;
    FeedManager *manager;
    bool active;
    LoadStatus status = LoadStatus::Idle;
};

ItemModel::ItemModel(QObject *parent) :
    QAbstractListModel(parent),
    priv(std::make_unique<PrivData>())
{

}

bool ItemModel::unreadFilter() const
{
    return priv->unreadFilter;
}

void ItemModel::setUnreadFilter(bool unreadFilter)
{
    if (priv->unreadFilter != unreadFilter) {
        priv->unreadFilter = unreadFilter;
        if (priv->active) {
            if (unreadFilter) refresh();
            else refreshMerge();
        }
        emit unreadFilterChanged();
    }
}

FeedManager *ItemModel::manager() const
{
    return priv->manager;
}

void ItemModel::setManager(FeedManager *manager)
{
    assert(priv->manager == nullptr);
    assert(manager != nullptr);
    priv->manager = manager;
    QObject::connect(manager, &FeedManager::itemAdded, this, &ItemModel::slotItemAdded);
    QObject::connect(manager, &FeedManager::itemChanged, this, &ItemModel::slotItemChanged);
    QObject::connect(manager, &FeedManager::feedStatusChanged, this, &ItemModel::slotFeedStatusChanged);
    if (priv->active) refresh();
    managerChanged();
}

LoadStatus ItemModel::status()
{
    return priv->status;
}

void ItemModel::setStatusFromUpstream()
{
    setStatus(LoadStatus::Idle);
}

void ItemModel::refresh()
{
    if (!priv->manager) return;
    setStatus(LoadStatus::Loading);
    priv->active = true;
    auto q = startQuery();
    QObject::connect(q, &FeedStorageOperation::finished, this, &ItemModel::slotQueryFinished);
}

void ItemModel::markAllRead()
{
    for (const auto &item : priv->items) {
        manager()->setRead(item.id, true);
    }
}


void ItemModel::classBegin()
{

}

void ItemModel::componentComplete()
{
    refresh();
}

ItemModel::~ItemModel() = default;

QHash<int, QByteArray> ItemModel::roleNames() const
{
    return {
         {Roles::Id, "id"},
         {Roles::Headline, "headline"},
         {Roles::Author, "author"},
         {Roles::Date, "date"},
         {Roles::Content, "content"},
         {Roles::Url, "url"},
         {Roles::IsUnread, "isUnread"},
         {Roles::IsStarred, "isStarred"}
    };
}

void ItemModel::slotQueryFinished()
{
    auto *q = static_cast<ItemQuery *>(QObject::sender());
    beginResetModel();
    priv->items = QList<StoredItem>::fromVector(q->result);
    endResetModel();
    setStatusFromUpstream();
}

void ItemModel::slotQueryFinishedMerge()
{
    auto *q = static_cast<ItemQuery *>(QObject::sender());
    auto &items = priv->items;
    int itemIndex = 0;
    for (const auto &resultItem : q->result) {
        while ((itemIndex < items.size()) && (items[itemIndex].headers.date > resultItem.headers.date)) {
            itemIndex++;
        }
        if (itemIndex >= items.size()) {
            insertAndNotify(items.size(), resultItem);
            itemIndex++;
        } else if (items[itemIndex].id != resultItem.id) {
            insertAndNotify(itemIndex, resultItem);
            itemIndex++;
        }
    }
}

inline qint64 indexForDate(const QList<StoredItem> &list, QDateTime dt)
{
    for (int i=0; i<list.count(); i++) {
        if (list.at(i).headers.date <= dt) return i;
    }
    return list.count();
}

void ItemModel::slotItemAdded(StoredItem const &item)
{
    if ((itemFilter(item))
         && (!priv->unreadFilter || !item.status.isRead))
    {
        auto idx = indexForDate(priv->items, item.headers.date);
        beginInsertRows(QModelIndex(), idx, idx);
        priv->items.insert(idx, item);
        endInsertRows();
    }
}

void ItemModel::slotItemChanged(StoredItem const &item)
{
    for(auto i=0; i<priv->items.size(); i++) {
        if (priv->items[i].id == item.id) {
            // changing the date interferes with the selection
            // when the list is sorted by date, so don't do that
            auto newItem = item;
            newItem.headers.date = priv->items[i].headers.date;

            priv->items[i] = newItem;
            auto idx = index(i);
            dataChanged(idx, idx);
            break;
        }
    }
}

void ItemModel::slotFeedStatusChanged(qint64 feedId, LoadStatus status)
{
    if (this->status() == LoadStatus::Loading) return;
    setStatusFromUpstream();
}

void ItemModel::setStatus(LoadStatus status)
{
    if (status != priv->status) {
        priv->status = status;
        emit statusChanged();
    }
}

void ItemModel::insertAndNotify(qint64 index, const StoredItem &item)
{
    beginInsertRows(QModelIndex(), index, index);
    priv->items.insert(index, item);
    endInsertRows();
}

void ItemModel::refreshMerge()
{
    auto q = startQuery();
    QObject::connect(q, &FeedStorageOperation::finished, this, &ItemModel::slotQueryFinishedMerge);
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return priv->items.size();
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int indexRow = index.row() ;

    switch(role){
        case Roles::Id:
            return priv->items[indexRow].id;

        case Roles::Headline:
            return priv->items[indexRow].headers.headline;

        case Roles::Author:
            return priv->items[indexRow].headers.author;

        case Roles::Date:
            return priv->items[indexRow].headers.date;

        case Roles::Content:
            return priv->items[indexRow].content;

        case Roles::Url:
            return priv->items[indexRow].headers.url;

        case Roles::IsUnread:
            return !priv->items[indexRow].status.isRead;

        case Roles::IsStarred:
            return priv->items[indexRow].status.isStarred;
    }

    return QVariant();
}
