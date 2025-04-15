/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QSortFilterProxyModel>

/**
 * Proxy model for the feed list that filters to show only editable feeds and sorts them by name
 *
 * Source model is expected to be a FeedListModel
 */
class EditableFeedListModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit EditableFeedListModel(QObject *parent = nullptr);

protected:
    // filter by editable feeds
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    // sort by feed name
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};
