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

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<StoredFeed> m_feeds;

public slots:
    void addFeed(StoredFeed const &feed);

};

#endif // FEEDLISTMODEL_H
