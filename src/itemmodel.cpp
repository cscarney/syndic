#include "itemmodel.h"

#include "articleref.h"
#include "context.h"
#include "feed.h"
#include "article.h"
#include "qmlarticleref.h"

using namespace FeedCore;

struct ItemModel::PrivData {
    QList<ArticleRef> items;
    bool unreadFilter = false;
    LoadStatus status = LoadStatus::Idle;
};

ItemModel::ItemModel(QObject *parent) :
    ManagedListModel(parent),
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
        if (active()) {
            if (unreadFilter) {
                refresh();
            } else {
                refreshMerge();
            }
        }
        emit unreadFilterChanged();
    }
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
    assert(manager() != nullptr);
    setStatus(LoadStatus::Loading);
    ItemQuery *q = startQuery();
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        reloadFromQuery(q);
    });
}

void ItemModel::markAllRead()
{
    const auto &items = priv->items;
    for (const auto &item : items) {
        item->setRead(true);
    }
}

ItemModel::~ItemModel() = default;

QHash<int, QByteArray> ItemModel::roleNames() const
{
    return {
         {Roles::Ref, "ref"}
    };
}

void ItemModel::reloadFromQuery(ItemQuery *query)
{
    beginResetModel();
    priv->items = QList<ArticleRef>::fromVector(query->result());
    endResetModel();
    setStatusFromUpstream();
}

void ItemModel::mergeFromQuery(ItemQuery *query)
{
    auto &items = priv->items;
    int itemIndex = 0;
    const auto &result = query->result();
    for (const auto &resultItem : result) {
        while ((itemIndex < items.size()) && (items[itemIndex]->date() > resultItem->date())) {
            itemIndex++;
        }
        if (itemIndex >= items.size()) {
            insertAndNotify(items.size(), resultItem);
            itemIndex++;
        } else if (items[itemIndex] != resultItem) {
            insertAndNotify(itemIndex, resultItem);
            itemIndex++;
        }
    }
}

inline qint64 indexForDate(const QList<ArticleRef> &list, const QDateTime &dt)
{
    for (int i=0; i<list.count(); i++) {
        if (list.at(i)->date() <= dt) {
            return i;
        }
    }
    return list.count();
}

void ItemModel::onItemAdded(ArticleRef const &item)
{
    if (!priv->unreadFilter || !item->isRead()) {
        const auto &idx = indexForDate(priv->items, item->date());
        beginInsertRows(QModelIndex(), idx, idx);
        priv->items.insert(idx, item);
        endInsertRows();
    }
}

void ItemModel::onItemChanged(ArticleRef const &item)
{
    const int size = priv->items.size();
    for(int i=0; i<size; i++) {
        ArticleRef &listItem = priv->items[i];
        if (listItem == item) {
            listItem = item;
            auto idx = index(i);
            emit dataChanged(idx, idx);
            break;
        }
    }
}

void ItemModel::setStatus(LoadStatus status)
{
    if (status != priv->status) {
        priv->status = status;
        emit statusChanged();
    }
}

void ItemModel::insertAndNotify(qint64 index, const ArticleRef &item)
{
    beginInsertRows(QModelIndex(), index, index);
    priv->items.insert(index, item);
    endInsertRows();
}

void ItemModel::refreshMerge()
{
    auto *q = startQuery();
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        mergeFromQuery(q);
    });
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return priv->items.size();
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int indexRow = index.row() ;

    if (role == Roles::Ref) {
        return QVariant::fromValue(QmlArticleRef(priv->items[indexRow]));
    }

    return QVariant();
}
