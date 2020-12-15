#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H

#include <memory>

#include "managedlistmodel.h"
#include "enums.h"
#include "feedref.h"

namespace FeedCore {
class StoredItem;
}

class FeedListModel : public ManagedListModel
{
    Q_OBJECT
public:
    explicit FeedListModel(QObject *parent = nullptr);
    ~FeedListModel();

    enum Roles {
        Ref = Qt::UserRole,
        Icon,
        UnreadCount
    };
    Q_ENUM(Roles);

    void initialize() override;

    // QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private slots:
    void slotFeedQueryFinished();
    void slotItemReadChanged(const FeedCore::StoredItem &item);
    void slotItemAdded(const FeedCore::StoredItem &item);
    void slotFeedAdded(const FeedCore::FeedRef &feed);

private:
    class PrivData;
    std::unique_ptr<PrivData> priv;
};

#endif // FEEDLISTMODEL_H
