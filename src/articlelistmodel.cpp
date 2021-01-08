#include "articlelistmodel.h"
#include <QDebug>
#include "feed.h"
#include "updater.h"
#include "articleref.h"
#include "qmlfeedref.h"
#include "qmlarticleref.h"

using namespace FeedCore;

struct ArticleListModel::PrivData {
    Feed *feed;
    QList<QmlArticleRef> items;
    bool unreadFilter { false };
    LoadStatus status { LoadStatus::Idle };
    bool active { false };
};

ArticleListModel::ArticleListModel(QObject *parent) :
    QAbstractListModel(parent),
    priv{ std::make_unique<PrivData>() }
{

}

bool ArticleListModel::unreadFilter() const
{
    return priv->unreadFilter;
}

void ArticleListModel::setUnreadFilter(bool unreadFilter)
{
    if (priv->unreadFilter != unreadFilter) {
        priv->unreadFilter = unreadFilter;
        if (priv->active) {
            if (unreadFilter) {
                refresh();
            } else {
                refreshMerge();
            }
        }
        emit unreadFilterChanged();
    }
}

LoadStatus ArticleListModel::status()
{
    return priv->status;
}

void ArticleListModel::refresh()
{
    setStatus(LoadStatus::Loading);
    auto *q = getItems();
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        onRefreshFinished(q);
    });
}

void ArticleListModel::markAllRead()
{
    const auto &items = priv->items;
    for (const auto &item : items) {
        item->setRead(true);
    }
}

ArticleListModel::~ArticleListModel() = default;

QHash<int, QByteArray> ArticleListModel::roleNames() const
{
    return {
         {Qt::UserRole, "ref"}
    };
}

void ArticleListModel::classBegin()
{

}

void ArticleListModel::componentComplete()
{
    QTimer::singleShot(0, this, [this]{
        qDebug() << "article list model complete";
        priv->active = true;
        QObject::connect(priv->feed, &Feed::articleAdded, this, &ArticleListModel::onItemAdded);
        QObject::connect(priv->feed, &Feed::statusChanged, this, &ArticleListModel::onStatusChanged);
        QObject::connect(priv->feed, &Feed::reset, this, &ArticleListModel::refresh);
        refresh();
    });
}

void ArticleListModel::onRefreshFinished(Future<ArticleRef> *sender)
{
    beginResetModel();
    priv->items = {};
    for (const ArticleRef &i : sender->result()) {
        priv->items.append(QmlArticleRef(i));
    }
    endResetModel();
    setStatusFromUpstream();
}

void ArticleListModel::onMergeFinished(Future<ArticleRef> *sender)
{
    auto &items = priv->items;
    int itemIndex = 0;
    const auto &result = sender->result();
    for (const auto &resultItem : result) {
        while ((itemIndex < items.size()) && (items[itemIndex]->date() > resultItem->date())) {
            itemIndex++;
        }
        if (itemIndex >= items.size()) {
            insertAndNotify(items.size(), resultItem);
            itemIndex++;
        } else if (items[itemIndex].get() != resultItem.get()) {
            insertAndNotify(itemIndex, resultItem);
            itemIndex++;
        }
    }
}

static qint64 indexForDate(const QList<QmlArticleRef> &list, const QDateTime &dt)
{
    for (int i=0; i<list.count(); i++) {
        if (list.at(i)->date() <= dt) {
            return i;
        }
    }
    return list.count();
}

void ArticleListModel::onItemAdded(ArticleRef const &item)
{
    if (!priv->unreadFilter || !item->isRead()) {
        const auto &idx = indexForDate(priv->items, item->date());
        beginInsertRows(QModelIndex(), idx, idx);
        priv->items.insert(idx, QmlArticleRef(item));
        endInsertRows();
    }
}

void ArticleListModel::setStatus(LoadStatus status)
{
    if (status != priv->status) {
        priv->status = status;
        emit statusChanged();
    }
}

void ArticleListModel::insertAndNotify(qint64 index, const ArticleRef &item)
{
    beginInsertRows(QModelIndex(), index, index);
    priv->items.insert(index, QmlArticleRef(item));
    endInsertRows();
}

void ArticleListModel::refreshMerge()
{
    auto *q = getItems();
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        onMergeFinished(q);
    });
}

int ArticleListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return priv->items.size();
}

QVariant ArticleListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int indexRow = index.row() ;

    if (role == Qt::UserRole) {
        return QVariant::fromValue(QmlArticleRef(priv->items[indexRow]));
    }

    return QVariant();
}

Feed *ArticleListModel::feed() const
{
    return priv->feed;
}

void ArticleListModel::setFeed(Feed *feed)
{
    if (priv->feed != feed) {
        priv->feed = feed;
        if (priv->active) {
            qDebug() << "set feed after initialization!!";
        }
        emit feedChanged();
    }
}

void ArticleListModel::requestUpdate() const
{
    feed()->updater()->start();
}

Future<ArticleRef> *ArticleListModel::getItems()
{
    if (priv->feed == nullptr) {
        qDebug() << "starting query with no feed set!";
        return nullptr;
    }
    return priv->feed->getArticles(unreadFilter());
}

void ArticleListModel::setStatusFromUpstream()
{
    setStatus(priv->feed->status());
}

void ArticleListModel::onStatusChanged()
{
    if (status() != Enums::Loading) {
        setStatus(priv->feed->status());
    }
}
