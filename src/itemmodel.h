#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H
#include <memory>
#include "managedlistmodel.h"
#include "enums.h"
#include "future.h"
namespace FeedCore {
class Context;
class ArticleRef;
}

class ItemModel : public ManagedListModel
{
    Q_OBJECT
public:
    explicit ItemModel(QObject *parent=nullptr);
    ~ItemModel();
    enum Roles {
        Ref = Qt::UserRole
    };
    Q_ENUM(Roles);
    bool unreadFilter() const;
    void setUnreadFilter(bool unreadFilter);
    FeedCore::LoadStatus status();
    Q_INVOKABLE virtual void requestUpdate() {}
    Q_INVOKABLE void markAllRead();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_PROPERTY(bool unreadFilter READ unreadFilter WRITE setUnreadFilter NOTIFY unreadFilterChanged);
    Q_PROPERTY(FeedCore::Enums::LoadStatus status READ status NOTIFY statusChanged);
signals:
    void unreadFilterChanged();
    void statusChanged();
protected:
    virtual FeedCore::Future<FeedCore::ArticleRef> *startQuery() = 0;
    virtual void setStatusFromUpstream();
    void setStatus(FeedCore::LoadStatus status);
    void refresh();
    void onItemAdded(const FeedCore::ArticleRef &item);
private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
    void removeRead();
    void insertAndNotify(qint64 index, const FeedCore::ArticleRef &item);
    void refreshMerge();
    void reloadFromQuery(FeedCore::Future<FeedCore::ArticleRef> *query);
    void mergeFromQuery(FeedCore::Future<FeedCore::ArticleRef> *query);
    void onItemChanged(const FeedCore::ArticleRef &item);
};
#endif // UNREADITEMMODEL_H
