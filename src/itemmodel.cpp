#include "itemmodel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "ttrssfeedsource.h"

ItemModel::ItemModel(QObject *parent, bool unreadFilter, std::optional<qint64> feedFilter)
    : QAbstractListModel(parent),
      m_feedFilter(feedFilter),
      m_unreadFilter(unreadFilter)
{

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
