/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feedlistmodel.h"
#include "context.h"
#include "iconprovider.h"
#include "starreditemsfeed.h"
#include <QList>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <algorithm>

using namespace FeedCore;

namespace
{
enum SpecialFeedIndex { ALL_ITEMS_IDX = 0, STARRED_ITEMS_IDX, SPECIAL_FEED_COUNT };

class SortHelper : public FeedSortNotifier
{
public:
    typedef bool (*Comparator)(const Feed *, const Feed *);
    const Comparator comparator;

    explicit SortHelper(Comparator comparator)
        : comparator{comparator} {};

    virtual void connectFeed(const Feed *feed) = 0;

    template<typename SlotType>
    void connectModel(FeedListModel *model, SlotType slot)
    {
        QObject::connect(this, &FeedSortNotifier::feedSortValueChanged, model, slot);
    }

    void onFeedSortValueChanged()
    {
        emit feedSortValueChanged(qobject_cast<Feed *>(QObject::sender()));
    }
};

class NameSortHelper : public SortHelper
{
public:
    NameSortHelper()
        : SortHelper(&cmp)
    {
    }

    static bool cmp(const Feed *left, const Feed *right)
    {
        int catComp = left->category().localeAwareCompare(right->category());
        return catComp < 0 || (catComp == 0 && left->name().localeAwareCompare(right->name()) < 0);
    };

    void connectFeed(const Feed *feed) final
    {
        QObject::connect(feed, &FeedCore::Feed::categoryChanged, this, &SortHelper::onFeedSortValueChanged);
        QObject::connect(feed, &FeedCore::Feed::nameChanged, this, &SortHelper::onFeedSortValueChanged);
    }
};

class UnreadCountSortHelper : public SortHelper
{
public:
    UnreadCountSortHelper()
        : SortHelper(&cmp){};

    static bool cmp(const Feed *left, const Feed *right)
    {
        int catComp = left->category().localeAwareCompare(right->category());
        if (catComp != 0) {
            return catComp < 0;
        }
        int unreadComp = right->unreadCount() - left->unreadCount();
        if (unreadComp != 0) {
            return unreadComp < 0;
        }
        return left->name().localeAwareCompare(right->name()) < 0;
    }

    void connectFeed(const Feed *feed) final
    {
        QObject::connect(feed, &FeedCore::Feed::categoryChanged, this, &SortHelper::onFeedSortValueChanged);
        QObject::connect(feed, &FeedCore::Feed::nameChanged, this, &SortHelper::onFeedSortValueChanged);
        QObject::connect(feed, &FeedCore::Feed::unreadCountChanged, this, &SortHelper::onFeedSortValueChanged);
    }
};
}

class FeedListModel::PrivData
{
public:
    FeedListModel *parent;
    Context *context = nullptr;
    QSharedPointer<Feed> allItems = nullptr;
    StarredItemsFeed *starredItems = nullptr;
    QList<FeedCore::Feed *> feeds;
    Sort sortMode = Name;
    std::unique_ptr<SortHelper> sortHelper = std::make_unique<NameSortHelper>();

    explicit PrivData(FeedListModel *parent)
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
    d->sortHelper->connectModel(this, &FeedListModel::onFeedSortValueChanged);
}

FeedListModel::~FeedListModel() = default;

void FeedListModel::PrivData::addItem(Feed *feed)
{
    addItem(feed, feeds.size());
}

void FeedListModel::PrivData::addItem(FeedCore::Feed *feed, int index)
{
    feeds.insert(index, feed);
    IconProvider::discoverIcon(feed);
    QObject::connect(feed, &QObject::destroyed, parent, [this, feed] {
        removeItem(feed);
    });
    sortHelper->connectFeed(feed);
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
        d->allItems = d->context->allItemsFeed();
        d->allItems->setName(tr("All Items", "special feed name"));
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
        entry = d->allItems.get();
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

    default:
        return QVariant();
    }
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
    QTimer::singleShot(0, this, [this] {
        loadFeeds();
        QObject::connect(d->context, &Context::feedAdded, this, &FeedListModel::onFeedAdded);
    });
}

FeedListModel::Sort FeedListModel::sortMode() const
{
    return d->sortMode;
}

void FeedListModel::setSortMode(FeedListModel::Sort sortMode)
{
    if (d->sortMode == sortMode) {
        return;
    }

    d->sortMode = sortMode;
    switch (sortMode) {
    case Name:
        d->sortHelper = std::make_unique<NameSortHelper>();
        break;

    case UnreadCount:
        d->sortHelper = std::make_unique<UnreadCountSortHelper>();
        break;
    }

    d->sortHelper->connectModel(this, &FeedListModel::onFeedSortValueChanged);
    emit sortModeChanged();
    if (!d->feeds.isEmpty()) {
        loadFeeds();
    }
}

int FeedListModel::indexOf(FeedCore::Feed *feed)
{
    int idx = d->feeds.indexOf(feed);
    return idx < 0 ? idx : idx + SPECIAL_FEED_COUNT;
}

void FeedListModel::loadFeeds()
{
    beginResetModel();
    d->feeds.clear();
    for (const auto &item : d->context->getFeeds()) {
        d->addItem(item);
    }
    std::sort(d->feeds.begin(), d->feeds.end(), d->sortHelper->comparator);
    endResetModel();
}

void FeedListModel::onFeedAdded(FeedCore::Feed *feed)
{
    const auto it = std::lower_bound(d->feeds.constBegin(), d->feeds.constEnd(), feed, d->sortHelper->comparator);
    const int index = int(it - d->feeds.constBegin());
    const int row = index + SPECIAL_FEED_COUNT;
    beginInsertRows(QModelIndex(), row, row);
    d->addItem(feed, index);
    endInsertRows();
}

void FeedListModel::onFeedSortValueChanged(Feed *feed)
{
    const auto comparator = d->sortHelper->comparator;
    const QList<Feed *>::iterator it = std::find(d->feeds.begin(), d->feeds.end(), feed);
    if (it == d->feeds.end()) {
        // feed not found
        return;
    }

    const QList<Feed *>::iterator next_it = it + 1;
    if (next_it != d->feeds.end() && comparator(*next_it, feed)) {
        // move toward end
        const QList<Feed *>::iterator newLocation = std::lower_bound(next_it, d->feeds.end(), feed, comparator);
        int oldRow = int(it - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        int newRow = int(newLocation - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
        std::rotate(it, it + 1, newLocation);
        endMoveRows();
        return;
    }

    const QList<Feed *>::iterator prev_it = it - 1;
    if (it != d->feeds.begin() && comparator(feed, *prev_it)) {
        // move toward beginning
        const QList<Feed *>::iterator newLocation = std::lower_bound(d->feeds.begin(), it, feed, comparator);
        int oldRow = int(it - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        int newRow = int(newLocation - d->feeds.begin()) + SPECIAL_FEED_COUNT;
        beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
        std::rotate(newLocation, it, it + 1);
        endMoveRows();
        return;
    }

    // no change in position
}
