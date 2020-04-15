#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H

#include <memory>
#include <optional>

#include <QAbstractListModel>
#include "feedsource.h"

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

signals:
    void itemMarked(FeedSource::Item const &item, FeedSource::ItemFlag flag, bool value);

public slots:
    void addItem(FeedSource::Item const &item);
    void updateItem(FeedSource::Item const &item);

private:
    QVector<FeedSource::Item> m_items;
    std::optional<qint64> m_feedFilter;
    bool m_unreadFilter;
};

#endif // UNREADITEMMODEL_H
