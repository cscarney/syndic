#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSortFilterProxyModel>

#include "itemmodel.h"
#include "feedmanager.h"
#include "feedstorage.h"

ItemModel::ItemModel(bool unreadFilter, std::optional<qint64> feedFilter, QObject *parent)
    : QAbstractListModel(parent),
      m_feedFilter(feedFilter),
      m_unreadFilter(unreadFilter)
{

}

void ItemModel::populate(FeedStorage *storage)
{
    auto *q = storage->startItemQuery(m_feedFilter, m_unreadFilter);
    QObject::connect(q, &FeedStorage::ItemQuery::finished, this, &ItemModel::slotQueryFinished);
}

void ItemModel::listenForUpdates(FeedManager *fm)
{
    QObject::connect(fm, &FeedManager::itemAdded, this, &ItemModel::addItem);
    QObject::connect(fm, &FeedManager::itemChanged, this,&ItemModel::updateItem);
}

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

void ItemModel::addItem(StoredItem const &item)
{
    if (((m_feedFilter==std::nullopt) || ((*m_feedFilter) == item.feedId))
         && (!m_unreadFilter || !item.status.isRead))
    {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append(item);
        endInsertRows();
    }
}

void ItemModel::updateItem(StoredItem const &item)
{
    for(auto i=0; i<m_items.size(); i++) {
        if (m_items[i].id == item.id) {

            // changing the date interferes with the selection
            // when the list is sorted by date, so don't do that
            auto newItem = item;
            newItem.headers.date = m_items[i].headers.date;

            m_items[i] = newItem;
            auto idx = index(i);
            dataChanged(idx, idx);
            break;
        }
    }
}

QAbstractItemModel *ItemModel::createSortedProxy()
{
    auto *sortedModel = new QSortFilterProxyModel;
    sortedModel->setSourceModel(this);
    sortedModel->setSortRole(ItemModel::Date);
    sortedModel->sort(0, Qt::DescendingOrder);
    setParent(sortedModel);
    return sortedModel;
}

void ItemModel::slotQueryFinished()
{
    auto *q = static_cast<FeedStorage::ItemQuery *>(QObject::sender());
    beginResetModel();
    for (auto const &storedItem : q->result) {
        m_items.append(storedItem);
    }
    endResetModel();
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int indexRow = index.row();
    switch(role){
        case Roles::Id:
            return m_items[indexRow].id;

        case Roles::Headline:
            return m_items[indexRow].headers.headline;

        case Roles::Author:
            return m_items[indexRow].headers.author;

        case Roles::Date:
            return m_items[indexRow].headers.date;

        case Roles::Content:
            return m_items[indexRow].content;

        case Roles::Url:
            return m_items[indexRow].headers.url;

        case Roles::IsUnread:
            return !m_items[indexRow].status.isRead;

        case Roles::IsStarred:
            return m_items[indexRow].status.isStarred;
    }

    return QVariant();
}
