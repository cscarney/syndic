/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "editablefeedlistmodel.h"
#include "feed.h"
#include "feedlistmodel.h"

EditableFeedListModel::EditableFeedListModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(Qt::DisplayRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::AscendingOrder);
}

bool EditableFeedListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto *feed = sourceModel()->data(index, FeedListModel::FeedRole).value<FeedCore::Feed *>();
    return (feed != nullptr) && feed->editable();
}

bool EditableFeedListModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // sort by feed name
    auto *leftFeed = sourceModel()->data(left, FeedListModel::FeedRole).value<FeedCore::Feed *>();
    auto *rightFeed = sourceModel()->data(right, FeedListModel::FeedRole).value<FeedCore::Feed *>();

    if (!leftFeed || !rightFeed) {
        return false;
    }
    return leftFeed->name().localeAwareCompare(rightFeed->name()) < 0;
}
