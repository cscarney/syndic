#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H

#include "managedlistmodel.h"
#include "storedfeed.h"
#include "feedmanager.h"


class FeedListModel : public ManagedListModel
{
    Q_OBJECT
public:
    explicit FeedListModel(QObject *parent = nullptr);
    ~FeedListModel();

    enum EntryType {
        SingleFeedType,
        AllType,
        StarredType,
        GroupType
    };
    Q_ENUM(EntryType);

    enum Roles {
        Id = Qt::UserRole,
        Type,
        Name,
        Icon,
        Status,
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
    void slotItemReadChanged(const StoredItem &item);
    void slotItemAdded(const StoredItem &item);
    void slotFeedStatusChanged(qint64 feedId, LoadStatus status);
    void slotFeedAdded(StoredFeed feed);
    void slotFeedNameChanged(qint64 feedId, QString newName);

private:
    class PrivData;
    std::unique_ptr<PrivData> priv;
};

#endif // FEEDLISTMODEL_H
