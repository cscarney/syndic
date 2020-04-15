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

void ItemModel::addItem(FeedSource::Item const &item)
{
    if (((m_feedFilter==std::nullopt) || ((*m_feedFilter) == item.feedId))
         && (!m_unreadFilter || item.isUnread))
    {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append(item);
        endInsertRows();
    }
}

void ItemModel::updateItem(const FeedSource::Item &item)
{
    for(auto i=0; i<m_items.size(); i++) {
        if (m_items[i].id == item.id) {
            m_items[i] = item;
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
            return m_items[indexRow].headline;

        case Roles::Author:
            return m_items[indexRow].author;

        case Roles::Date:
            return m_items[indexRow].date;

        case Roles::Content:
            return m_items[indexRow].content;

        case Roles::Url:
            return m_items[indexRow].url;

        case Roles::IsUnread:
            return m_items[indexRow].isUnread;

        case Roles::IsStarred:
            return m_items[indexRow].isStarred;
    }

    return QVariant();
}
