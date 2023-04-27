/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "articleref.h"
#include "feed.h"
#include "future.h"
#include <QModelIndex>
#include <QQmlParserStatus>
#include <memory>
namespace FeedCore
{
class Context;
}

/**
 * List model for the article list.
 *
 * The model is tied to a single feed.  To display a combined list of feeds,
 * the model can be populated from an AllItemsFeed.
 */
class ArticleListModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    /**
     * Set to true to filter out unread articles.
     *
     * Note that this filtering is not dynamic -- an article hat is unread when loaded into the model remains
     * in the list until removeRead() is called.  This prevents articles being removed from the list  while
     * they are still open.
     */
    Q_PROPERTY(bool unreadFilter READ unreadFilter WRITE setUnreadFilter NOTIFY unreadFilterChanged);

    /**
     * A convenience accessor for feed()->status()
     */
    Q_PROPERTY(FeedCore::Feed::LoadStatus status READ status NOTIFY statusChanged);

    /**
     * The feed whose items are displayed in the list
     */
    Q_PROPERTY(FeedCore::Feed *feed READ feed WRITE setFeed NOTIFY feedChanged);

public:
    explicit ArticleListModel(QObject *parent = nullptr);
    ~ArticleListModel();

    /**
     * Begins an update on the feed and updates the list accordingly.
     */
    Q_INVOKABLE void requestUpdate();

    /**
     * Marks all of the items displayed in the list as read and removes
     * them from the list if necessary.
     */
    Q_INVOKABLE void markAllRead();

    /**
     * If unreadFilter is true, this re-filters the list.
     */
    Q_INVOKABLE void removeRead();

    FeedCore::Feed *feed() const;
    void setFeed(FeedCore::Feed *feed);
    bool unreadFilter() const;
    void setUnreadFilter(bool unreadFilter);
    FeedCore::LoadStatus status();

    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    QHash<int, QByteArray> roleNames() const override;
    void classBegin() override;
    void componentComplete() override;

signals:
    void feedChanged();
    void unreadFilterChanged();
    void statusChanged();

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    template<typename Callback>
    void getItems(Callback cb);
    void setStatusFromUpstream();
    void setStatus(FeedCore::LoadStatus status);
    void refresh();
    void onItemAdded(const FeedCore::ArticleRef &item);
    void insertAndNotify(int index, const FeedCore::ArticleRef &item);
    void refreshMerge();
    void onRefreshFinished(const QList<FeedCore::ArticleRef> &result);
    void onMergeFinished(const QList<FeedCore::ArticleRef> &result);
    void onStatusChanged();
    class RowRemoveHelper;
};
