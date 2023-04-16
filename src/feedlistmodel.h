/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "articleref.h"
#include "context.h"
#include "future.h"
#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <memory>
namespace FeedCore
{
class Feed;
}

/**
 * List model for the main list of feeds
 */
class FeedListModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(FeedCore::Context *context READ context WRITE setContext NOTIFY contextChanged);
    Q_PROPERTY(Sort sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged);

public:
    enum Roles { FeedRole = Qt::UserRole, CategoryRole };
    Q_ENUM(Roles);
    enum Sort { Name, UnreadCount };
    Q_ENUM(Sort);
    explicit FeedListModel(QObject *parent = nullptr);
    ~FeedListModel();
    FeedCore::Context *context() const;
    void setContext(FeedCore::Context *context);
    Sort sortMode() const;
    void setSortMode(Sort sortMode);
    Q_INVOKABLE int indexOf(FeedCore::Feed *feed);

    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    QHash<int, QByteArray> roleNames() const final;
    void classBegin() override;
    void componentComplete() override;

signals:
    void contextChanged();
    void sortModeChanged();

private:
    class PrivData;
    std::unique_ptr<PrivData> d;
    void loadFeeds();
    void onFeedAdded(FeedCore::Feed *feed);
    void onFeedSortValueChanged(FeedCore::Feed *feed);
};

class FeedSortNotifier : public QObject
{
    Q_OBJECT
    using QObject::QObject;

signals:
    void feedSortValueChanged(FeedCore::Feed *feed);
};
