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
 * This is a generic list model for displaying a list of articles. It is
 * intended to be used as a base class for other models that display
 * articles from a feed or search result.
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
     * The current load status of the article list. This will be
     * Feed::Loading until the model finishes loading, otherwise
     * it takes it's value from the source.
     *
     * \sa FeedCore::Feed::LoadStatus
     * \sa ArticleListModel::setStatusFromUpstream
     */
    Q_PROPERTY(FeedCore::Feed::LoadStatus status READ status NOTIFY statusChanged);

public:
    explicit ArticleListModel(QObject *parent = nullptr);
    ~ArticleListModel();

    /**
     * Requests an update on the underlying source. This corresponds
     * to the user clicking the refresh button in the user interface.
     *
     * This function may be implemented by a derived class; the default
     * implementation does nothing.
     */
    Q_INVOKABLE virtual void requestUpdate();

    /**
     * Marks all of the items displayed in the list as read and removes
     * them from the list if necessary.
     */
    Q_INVOKABLE void markAllRead();

    /**
     * If unreadFilter is true, this re-filters the list.
     */
    Q_INVOKABLE void removeRead();

    bool unreadFilter() const;
    void setUnreadFilter(bool unreadFilter);
    FeedCore::LoadStatus status();

    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    QHash<int, QByteArray> roleNames() const override;
    void classBegin() override;
    void componentComplete() override;

signals:
    void unreadFilterChanged();
    void statusChanged();

protected:
    /**
     * Called before the first update to perform initial setup.
     *
     * The default implementation does nothing.
     */
    virtual void init();

    /**
     * Called by refresh() to get the list of items from the source.
     */
    virtual QFuture<FeedCore::ArticleRef> getArticles() = 0;

    /**
     * Called after an update to sync the load status of the model
     * with the load status of the source.
     *
     * The default implementation sets the status to Idle.
     */
    virtual void setStatusFromUpstream();

    typedef bool (*ArticleComparator)(const FeedCore::ArticleRef &, const FeedCore::ArticleRef &);
    virtual ArticleComparator getArticleComparator();

    /**
     * Returns true if the source has been initialized
     */
    bool active();

    /**
     * Remove all items
     */
    void clear();

    /**
     * Reloads the list from the source. This does *not* update
     * the source itself, it just replaces the existing content
     * of the list with the result of a new call to getArticles().
     */
    void refresh();

    /**
     * Add an item to the list.
     */
    void addItem(const FeedCore::ArticleRef &item);

    void setStatus(FeedCore::LoadStatus status);

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    template<typename Callback>
    void getItems(Callback cb);
    void insertAndNotify(int index, const FeedCore::ArticleRef &item);
    void refreshMerge();
    void onRefreshFinished(const QList<FeedCore::ArticleRef> &result);
    void onMergeFinished(const QList<FeedCore::ArticleRef> &result);
    void onStatusChanged();
    int indexForItem(const FeedCore::ArticleRef &item);
    class RowRemoveHelper;
};
