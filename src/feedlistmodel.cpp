/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feedlistmodel.h"
#include <algorithm>
#include <QString>
#include <QVector>
#include <QPointer>
#include "context.h"
#include "allitemsfeed.h"
#include "starreditemsfeed.h"
#include "iconprovider.h"
#include "networkaccessmanager.h"

using namespace FeedCore;

namespace  {
enum SpecialFeedIndex {
    ALL_ITEMS_IDX = 0,
    STARRED_ITEMS_IDX,
    SPECIAL_FEED_COUNT
};
}

class FeedListModel::PrivData {
public:
    FeedListModel *parent;
    Context *context = nullptr;
    AllItemsFeed *allItems = nullptr;
    StarredItemsFeed *starredItems = nullptr;
    QVector<FeedCore::Feed*> feeds;

    PrivData(FeedListModel *parent) :
        parent{ parent }
    {}

    void addItem(FeedCore::Feed *feed);
    void addItem(FeedCore::Feed *feed, int index);
    void removeItem(FeedCore::Feed *feed);
};

FeedListModel::FeedListModel(QObject *parent)
    : QAbstractListModel(parent),
      d{ std::make_unique<PrivData>(this) }
{
}

FeedListModel::~FeedListModel()=default;

void FeedListModel::PrivData::addItem(Feed *feed)
{
    addItem(feed, feeds.size());
}

void FeedListModel::PrivData::addItem(FeedCore::Feed *feed, int index)
{
    feeds.insert(index, feed);
    if (feed->icon().isEmpty()) {
        IconProvider::discoverIcon(feed);
    }
    QObject::connect(feed, &QObject::destroyed, parent, [this, feed]{
        removeItem(feed);
    });
}

void FeedListModel::PrivData::removeItem(FeedCore::Feed *feed)
{
    int i = 0;
    while (i < feeds.length()) {
        FeedCore::Feed *const candidate { feeds[i] };
        if (candidate==feed) {
            parent->beginRemoveRows(QModelIndex(), i, i);
            feeds.remove(i);
            parent->endRemoveRows();
        } else {
            ++i;
        }
    }
}

Context *FeedListModel::context() const
{
    return d->context;
}

void FeedListModel::setContext(FeedCore::Context *context)
{
    d->context = context;
    if (context != nullptr) {
        d->allItems = new AllItemsFeed(d->context, tr("All Items", "special feed name"), this);
        d->starredItems = new StarredItemsFeed(d->context, tr("Starred", "special feed name"), this);
    }
    emit contextChanged();
}

int FeedListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->feeds.size() + SPECIAL_FEED_COUNT;
}

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int indexRow = index.row();
    FeedCore::Feed *entry;
    switch (indexRow) {
    case ALL_ITEMS_IDX:
        entry = d->allItems;
        break;
    case STARRED_ITEMS_IDX:
        entry = d->starredItems;
        break;
    default:
        entry = d->feeds[indexRow-SPECIAL_FEED_COUNT];
    }

    return QVariant::fromValue(entry);
}


QHash<int, QByteArray> FeedListModel::roleNames() const
{
    return {
        {Qt::UserRole, "feed"}
    };
}

void FeedListModel::classBegin()
{

}

void FeedListModel::componentComplete()
{
    QTimer::singleShot(0, this, &FeedListModel::loadFeeds);
}

static bool compareFeedNames(Feed *left, Feed *right) {
    return left->name() < right->name();
}

void FeedListModel::loadFeeds()
{
    beginResetModel();
    d->feeds.clear();
    for (const auto &item : d->context->getFeeds()){
        d->addItem(item);
    }
    std::sort(d->feeds.begin(), d->feeds.end(), compareFeedNames);
    endResetModel();
    QObject::connect(d->context, &Context::feedAdded, this, &FeedListModel::onFeedAdded);
}

void FeedListModel::onFeedAdded(FeedCore::Feed *feed)
{
    auto it = std::lower_bound(d->feeds.constBegin(), d->feeds.constEnd(), feed, compareFeedNames);
    const int index = it - d->feeds.constBegin();
    const int row = index + SPECIAL_FEED_COUNT;
    beginInsertRows(QModelIndex(), row, row);
    d->addItem(feed, index);
    endInsertRows();
}
