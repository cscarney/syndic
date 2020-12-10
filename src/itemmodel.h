#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H

#include <memory>

#include "managedlistmodel.h"
#include "enums.h"
#include "feedstorageoperation.h"

namespace FeedCore {
class Context;
class StoredItem;
}

class ItemModel : public ManagedListModel
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit ItemModel(QObject *parent=nullptr);

    ~ItemModel();

    enum Roles {
        Id = Qt::UserRole,
        Headline,
        Author,
        Date,
        Content,
        Url,
        IsUnread,
        IsStarred
    };
    Q_ENUM(Roles);

    void initialize() override;

    bool unreadFilter() const;
    void setUnreadFilter(bool unreadFilter);
    Q_PROPERTY(bool unreadFilter READ unreadFilter WRITE setUnreadFilter NOTIFY unreadFilterChanged);

    FeedCore::LoadStatus status();
    Q_PROPERTY(FeedCore::Enums::LoadStatus status READ status NOTIFY statusChanged);

    Q_INVOKABLE virtual void requestUpdate() {}
    Q_INVOKABLE void markAllRead();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private slots:
    void slotQueryFinished();
    void slotQueryFinishedMerge();
    void slotItemAdded(const FeedCore::StoredItem &item);
    void slotItemChanged(const FeedCore::StoredItem &item);
    void slotFeedStatusChanged(const FeedCore::FeedRef &feed, FeedCore::Enums::LoadStatus status);

signals:
    void unreadFilterChanged();
    void statusChanged();

protected:
    virtual FeedCore::ItemQuery *startQuery() = 0;
    virtual bool itemFilter(const FeedCore::StoredItem &item) = 0;
    virtual void setStatusFromUpstream();
    void setStatus(FeedCore::LoadStatus status);
    void refresh();

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;

    void removeRead();
    void insertAndNotify(qint64 index, const FeedCore::StoredItem &item);
    void refreshMerge();
};

#endif // UNREADITEMMODEL_H
