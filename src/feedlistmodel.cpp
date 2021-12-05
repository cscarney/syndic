/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feedlistmodel.h"
#include "allitemsfeed.h"
#include "context.h"
#include "iconprovider.h"
#include "networkaccessmanager.h"
#include "starreditemsfeed.h"
#include <QPointer>
#include <QString>
#include <QVector>
#include <algorithm>

using namespace FeedCore;

namespace
{
enum SpecialFeedIndex { ALL_ITEMS_IDX = 0, STARRED_ITEMS_IDX, SPECIAL_FEED_COUNT };
}

class FeedListModel::PrivData
{
public:
    FeedListModel *parent;
    Context *context = nullptr;
    AllItemsFeed *allItems = nullptr;
    StarredItemsFeed *starredItems = nullptr;
    QVector<FeedCore::Feed *> feeds;

    PrivData(FeedListModel *parent)
        : parent{parent}
    {
    }

    void addItem(FeedCore::Feed *feed);
    void addItem(FeedCore::Feed *feed, int index);
    void removeItem(FeedCore::Feed *feed);
};

FeedListModel::FeedListModel(QObject *parent)
    : QAbstractListModel(parent)
    , d{std::make_unique<PrivData>(this)}
{
}

FeedListModel::~FeedListModel() = default;

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
    QObject::connect(feed, &QObject::destroyed, parent, [this, feed] {
        removeItem(feed);
    });
    QObject::connect(feed, &FeedCore::Feed::categoryChanged, parent, &FeedListModel::onFeedSortValueChanged);
    QObject::connect(feed, &FeedCore::Feed::nameChanged, parent, &FeedListModel::onFeedSortValueChanged);
}

void FeedListModel::PrivData::removeItem(FeedCore::Feed *feed)
{
    int i = feeds.indexOf(feed);
    if (i < 0) {
        return;
    }
    const int row = i + SPECIAL_FEED_COUNT;
    parent->beginRemoveRows(QModelIndex(), row, row);
    feeds.remove(i);
    parent->endRemoveRows();
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
    if (Q_UNLIKELY(!index.isValid())) {
        return QVariant();
    }

    const int indexRow = index.row();
    FeedCore::Feed *entry{nullptr};
    switch (indexRow) {
    case ALL_ITEMS_IDX:
        entry = d->allItems;
        break;
    case STARRED_ITEMS_IDX:
        entry = d->starredItems;
        break;
    default:
        entry = d->feeds[indexRow - SPECIAL_FEED_COUNT];
    }

    switch (role) {
    case FeedRole:
        return QVariant::fromValue(entry);

    case CategoryRole:
        return entry->category();
    }

    return QVariant();
}

QHash<int, QByteArray> FeedListModel::roleNames() const
{
    return {{FeedRole, "feed"}, {CategoryRole, "category"}};
}

void FeedListModel::classBegin()
{
}

void FeedListModel::componentComplete()
{
    QTimer::singleShot(0, this, &FeedListModel::loadFeeds);
}

static bool compareFeedNames(const Feed *left, const Feed *right)
{
    return std::pair(left->category(), left->name()) < std::pair(right->category(), right->name());
}

void FeedListModel::loadFeeds()
{
    beginResetModel();
    d->feeds.clear();
    for (const auto &item : d->context->getFeeds()) {
        d->addItem(item);
    }
    std::sort(d->feeds.begin(), d->feeds.end(), compareFeedNames);
    endResetModel();
    QObject::connect(d->context, &Context::feedAdded, this, &FeedListModel::onFeedAdded);
}

void FeedListModel::onFeedAdded(FeedCore::Feed *feed)
{
    const auto *it = std::lower_bound(d->feeds.constBegin(), d->feeds.constEnd(), feed, compareFeedNames);
    const int index = int(it - d->feeds.constBegin());
    const int row = index + SPECIAL_FEED_COUNT;
    beginInsertRows(QModelIndex(), row, row);
    d->addItem(feed, index);
    endInsertRows();
}

void FeedListModel::onFeedSortValueChanged()
{
    auto *feed = qobject_cast<FeedCore::Feed *>(QObject::sender());
    const QVector<Feed *>::iterator it = std::find(d->feeds.begin(), d->feeds.end(), feed);
    if (it == d->feeds.end()) {
        // feed not found
        return;
    }

    const QVector<Feed *>::iterator next_it = it + 1;
    if (next_it != d->feeds.end() && compareFeedNames(*next_it, feed)) {
        // move toward end
        const QVector<Feed *>::iterator newLocation = std::lower_bound(next_it, d->feeds.end(), feed, compareFeedNames);
        int oldRow = int(it - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        int newRow = int(newLocation - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
        std::rotate(it, it + 1, newLocation);
        endMoveRows();
        return;
    }

    const QVector<Feed *>::iterator prev_it = it - 1;
    if (it != d->feeds.begin() && compareFeedNames(feed, *prev_it)) {
        // move toward beginning
        const QVector<Feed *>::iterator newLocation = std::lower_bound(d->feeds.begin(), it, feed, compareFeedNames);
        int oldRow = int(it - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        int newRow = int(newLocation - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
        std::rotate(newLocation, it, it + 1);
        endMoveRows();
        return;
    }

    // no change in position
}
