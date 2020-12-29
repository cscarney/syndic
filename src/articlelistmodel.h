#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H
#include <memory>
#include "managedlistmodel.h"
#include "enums.h"
#include "future.h"
#include "feedref.h"
namespace FeedCore {
class Context;
class ArticleRef;
}

class ArticleListModel : public ManagedListModel
{
    Q_OBJECT
    Q_PROPERTY(bool unreadFilter READ unreadFilter WRITE setUnreadFilter NOTIFY unreadFilterChanged);
    Q_PROPERTY(FeedCore::Enums::LoadStatus status READ status NOTIFY statusChanged);
    Q_PROPERTY(FeedCore::FeedRef feed READ feed WRITE setFeed NOTIFY feedChanged);
public:
    explicit ArticleListModel(QObject *parent=nullptr);
    ~ArticleListModel();
    FeedCore::FeedRef feed() const;
    void setFeed(const FeedCore::FeedRef &feedId);
    bool unreadFilter() const;
    void setUnreadFilter(bool unreadFilter);
    FeedCore::LoadStatus status();
    Q_INVOKABLE void requestUpdate() const;
    Q_INVOKABLE void markAllRead();
    void initialize() final;
    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    QHash<int, QByteArray> roleNames() const override;
signals:
    void feedChanged();
    void unreadFilterChanged();
    void statusChanged();
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
    FeedCore::Future<FeedCore::ArticleRef> *getItems();
    void setStatusFromUpstream();
    void setStatus(FeedCore::LoadStatus status);
    void refresh();
    void onItemAdded(const FeedCore::ArticleRef &item);
    void removeRead();
    void insertAndNotify(qint64 index, const FeedCore::ArticleRef &item);
    void refreshMerge();
    void onRefreshFinished(FeedCore::Future<FeedCore::ArticleRef> *sender);
    void onMergeFinished(FeedCore::Future<FeedCore::ArticleRef> *sender);
    void onItemChanged(const FeedCore::ArticleRef &item);
    void onStatusChanged();
};
#endif // UNREADITEMMODEL_H
