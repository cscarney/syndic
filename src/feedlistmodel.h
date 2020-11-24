#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H

#include <QAbstractListModel>
#include "storedfeed.h"

class FeedListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FeedListModel(QObject *parent = nullptr);

    enum Roles {
        Id = Qt::UserRole,
        Name,
        UnreadCount
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void addFeed(StoredFeed const &feed);

private:
    QVector<StoredFeed> m_feeds;
};

#endif // FEEDLISTMODEL_H
