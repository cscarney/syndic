/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "articlelistmodel.h"
#include "articleref.h"
#include "feed.h"
#include "qmlarticleref.h"
#include <algorithm>

using namespace FeedCore;

struct ArticleListModel::PrivData {
    Feed *feed{};
    QList<QmlArticleRef> items;
    bool unreadFilter{false};
    LoadStatus status{LoadStatus::Idle};
    bool active{false};
};

ArticleListModel::ArticleListModel(QObject *parent)
    : QAbstractListModel(parent)
    , d{std::make_unique<PrivData>()}
{
}

bool ArticleListModel::unreadFilter() const
{
    return d->unreadFilter;
}

void ArticleListModel::setUnreadFilter(bool unreadFilter)
{
    if (d->unreadFilter != unreadFilter) {
        d->unreadFilter = unreadFilter;
        if (d->active) {
            if (unreadFilter) {
                removeRead();
            } else {
                refreshMerge();
            }
        }
        emit unreadFilterChanged();
    }
}

LoadStatus ArticleListModel::status()
{
    return d->status;
}

void ArticleListModel::refresh()
{
    setStatus(LoadStatus::Loading);
    auto *q = getItems();
    QObject::connect(q, &BaseFuture::finished, this, [this, q] {
        onRefreshFinished(q);
    });
}

void ArticleListModel::markAllRead()
{
    const auto &items = d->items;
    for (const auto &item : items) {
        item->setRead(true);
    }
    removeRead();
}

ArticleListModel::~ArticleListModel() = default;

QHash<int, QByteArray> ArticleListModel::roleNames() const
{
    return {{Qt::UserRole, "ref"}};
}

void ArticleListModel::classBegin()
{
}

void ArticleListModel::componentComplete()
{
    QTimer::singleShot(0, this, [this] {
        d->active = true;
        QObject::connect(d->feed, &Feed::articleAdded, this, &ArticleListModel::onItemAdded);
        QObject::connect(d->feed, &Feed::statusChanged, this, &ArticleListModel::onStatusChanged);
        QObject::connect(d->feed, &Feed::reset, this, &ArticleListModel::refresh);
        refresh();
    });
}

void ArticleListModel::onRefreshFinished(Future<ArticleRef> *sender)
{
    beginResetModel();
    d->items = {};
    for (const ArticleRef &i : sender->result()) {
        d->items.append(QmlArticleRef(i));
    }
    endResetModel();
    setStatusFromUpstream();
}

static bool compareDatesDescending(const ArticleRef &l, const ArticleRef &r)
{
    return l->date() > r->date();
}

static int indexForItem(const QList<QmlArticleRef> &list, const ArticleRef &item)
{
    auto it = std::lower_bound(list.constBegin(), list.constEnd(), item, compareDatesDescending);
    return it - list.constBegin();
}

void ArticleListModel::onMergeFinished(Future<ArticleRef> *sender)
{
    auto &items = d->items;
    QSet<Article *> knownItems(items.constBegin(), items.constEnd());
    for (const auto &item : sender->result()) {
        if (!knownItems.contains(item.get())) {
            insertAndNotify(indexForItem(d->items, item), item);
        }
    }
}

void ArticleListModel::onItemAdded(ArticleRef const &item)
{
    if (!d->unreadFilter || !item->isRead()) {
        insertAndNotify(indexForItem(d->items, item), item);
    }
}

void ArticleListModel::removeRead()
{
    if (!d->unreadFilter) {
        return;
    }
    auto &items = d->items;
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i)->isRead()) {
            beginRemoveRows(QModelIndex(), i, i);
            items.removeAt(i);
            endRemoveRows();
            --i;
        }
    }
}

void ArticleListModel::setStatus(LoadStatus status)
{
    if (status != d->status) {
        d->status = status;
        emit statusChanged();
    }
}

void ArticleListModel::insertAndNotify(int index, const ArticleRef &item)
{
    beginInsertRows(QModelIndex(), index, index);
    d->items.insert(index, QmlArticleRef(item));
    endInsertRows();
}

void ArticleListModel::refreshMerge()
{
    auto *q = getItems();
    QObject::connect(q, &BaseFuture::finished, this, [this, q] {
        onMergeFinished(q);
    });
}

int ArticleListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->items.size();
}

QVariant ArticleListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int indexRow = index.row();

    if (role == Qt::UserRole) {
        return QVariant::fromValue(QmlArticleRef(d->items[indexRow]));
    }

    return QVariant();
}

Feed *ArticleListModel::feed() const
{
    return d->feed;
}

void ArticleListModel::setFeed(Feed *feed)
{
    if (d->feed != feed) {
        d->feed = feed;
        if (d->active) {
            beginResetModel();
            d->items = {};
            endResetModel();
            refresh();
        }
        emit feedChanged();
    }
}

void ArticleListModel::requestUpdate()
{
    feed()->updater()->start();
    removeRead();
}

Future<ArticleRef> *ArticleListModel::getItems()
{
    if (d->feed == nullptr) {
        return Future<ArticleRef>::yield(this, [](auto /*unused*/) {});
    }
    return d->feed->getArticles(unreadFilter());
}

void ArticleListModel::setStatusFromUpstream()
{
    auto *feed = d->feed;
    if (feed != nullptr) {
        setStatus(feed->status());
    }
}

void ArticleListModel::onStatusChanged()
{
    if (status() != Feed::Loading) {
        setStatus(d->feed->status());
    }
}
