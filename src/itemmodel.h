#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H

#include <memory>
#include <optional>

#include <QAbstractListModel>
#include "storeditem.h"

class FeedManager;
class FeedStorage;

class ItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ItemModel(bool unreadFilter=false, std::optional<qint64> feedFilter=std::nullopt, QObject *parent=nullptr);

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

    void populate(FeedStorage *storage);
    void listenForUpdates(FeedManager *fm);
    void addItem(StoredItem const &item);
    void updateItem(StoredItem const &item);

    /* Creates a proxy model which sorts by date descending.
     * The newly created proxy model takes ownership of the ItemModel and the
     * caller takes ownership of the proxy. */
    QAbstractItemModel *createSortedProxy();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void slotQueryFinished();

private:
    QVector<StoredItem> m_items;
    std::optional<qint64> m_feedFilter;
    bool m_unreadFilter;
};

#endif // UNREADITEMMODEL_H
