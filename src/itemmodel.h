#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H

#include <memory>
#include <optional>

#include <QAbstractListModel>
#include "storeditem.h"

class ItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ItemModel(QObject *parent = nullptr, bool unreadFilter=false, std::optional<qint64> feedFilter=std::nullopt);

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

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

public slots:
    void addItem(StoredItem const &item);
    void updateItem(StoredItem const &item);

private:
    QVector<StoredItem> m_items;
    std::optional<qint64> m_feedFilter;
    bool m_unreadFilter;
};

#endif // UNREADITEMMODEL_H
