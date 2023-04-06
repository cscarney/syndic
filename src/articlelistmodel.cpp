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
    LoadStatus status{LoadStatus::Loading};
    bool active{false};
};

// helper class for batching row removals
class ArticleListModel::RowRemoveHelper
{
    int first = -1;
    int last = -1;

    bool extend(int index)
    {
        if (isEmpty()) {
            first = last = index;
        } else if (index == last + 1) {
            last = index;
        } else if (index == first - 1) {
            first = index;
        } else {
            return false;
        }
        return true;
    }

    void clear()
    {
        first = last = -1;
    }

    bool isEmpty() const
    {
        return first == -1 || last == -1;
    }

    int flush(ArticleListModel *model)
    {
        if (isEmpty()) {
            return 0;
        }
        int length = last - first + 1;
        model->beginRemoveRows(QModelIndex(), first, last);
        auto &items = model->d->items;
        items.erase(items.begin() + first, items.begin() + last + 1);
        model->endRemoveRows();
        clear();
        return length;
    }

public:
    template<typename WhereFunc>
    void removeWhere(ArticleListModel *model, WhereFunc cb)
    {
        auto &items = model->d->items;
        for (int i = 0; i < items.size(); ++i) {
            if (cb(items.at(i)) && !extend(i)) {
                i -= flush(model);
                extend(i);
            }
        }
        flush(model);
    }
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
    getItems([this](auto result) {
        onRefreshFinished(result);
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

void ArticleListModel::onRefreshFinished(const QVector<ArticleRef> &result)
{
    beginResetModel();
    d->items = {};
    for (const ArticleRef &i : result) {
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

void ArticleListModel::onMergeFinished(const QVector<ArticleRef> &result)
{
    auto &items = d->items;
    QSet<Article *> knownItems(items.constBegin(), items.constEnd());
    for (const auto &item : result) {
        if (!knownItems.contains(item.get())) {
            insertAndNotify(indexForItem(d->items, item), item);
        }
    }
    setStatusFromUpstream();
}

void ArticleListModel::onItemAdded(ArticleRef const &item)
{
    if (!d->unreadFilter || !item->isRead()) {
        insertAndNotify(indexForItem(d->items, item), item);
    }
}

void ArticleListModel::removeRead()
{
    setStatus(Feed::Loading);
    if (d->unreadFilter) {
        RowRemoveHelper helper;
        helper.removeWhere(this, [](const auto &item) {
            return item->isRead();
        });
    }
    setStatusFromUpstream();
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
    setStatus(Feed::Loading);
    getItems([this](auto result) {
        onMergeFinished(result);
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

static void removeReadArticles(QVector<ArticleRef> &v)
{
    auto *it = std::remove_if(v.begin(), v.end(), [](const ArticleRef &i) {
        return i->isRead();
    });
    v.erase(it, v.end());
}

template<typename Callback>
void ArticleListModel::getItems(Callback cb)
{
    if (!d->feed) {
        cb(QVector<ArticleRef>{});
        return;
    }
    Future<ArticleRef> *q = d->feed->getArticles(unreadFilter());
    QObject::connect(q, &BaseFuture::finished, this, [this, cb, q] {
        QVector result = q->result();
        if (unreadFilter()) {
            removeReadArticles(result);
        }
        std::sort(result.begin(), result.end(), &compareDatesDescending);
        cb(result);
    });
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
