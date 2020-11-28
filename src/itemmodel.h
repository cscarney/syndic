#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H

#include <memory>
#include <optional>

#include <QAbstractListModel>
#include <QQmlParserStatus>

#include "storeditem.h"

class FeedManager;
class FeedStorage;

class ItemModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit ItemModel(QObject *parent=nullptr);
    explicit ItemModel(FeedManager *manager, bool unreadFilter=false, std::optional<qint64> feedFilter=std::nullopt, QObject *parent=nullptr);

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

    Q_INVOKABLE bool unreadFilter() const;
    Q_INVOKABLE void setUnreadFilter(bool unreadFilter);
    Q_PROPERTY(bool unreadFilter READ unreadFilter WRITE setUnreadFilter NOTIFY unreadFilterChanged)

    std::optional<qint64> feedFilter() const;
    void setFeedFilter(std::optional<qint64> feedFilter);
    Q_INVOKABLE void setFeedFilter(QVariant feedFilter);
    Q_INVOKABLE QVariant feedFilterVariant() const;
    Q_PROPERTY(QVariant feedFilter READ feedFilterVariant WRITE setFeedFilter NOTIFY feedFilterChanged);

    Q_INVOKABLE FeedManager *manager() const;
    Q_INVOKABLE void setManager(FeedManager *manager);
    Q_PROPERTY(FeedManager *manager READ manager WRITE setManager NOTIFY managerChanged);

    void refresh();
    void classBegin() override;
    void componentComplete() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void unreadFilterChanged();
    void feedFilterChanged();
    void managerChanged();

public slots:
    void slotQueryFinished();
    void slotQueryFinishedMerge();
    void slotItemAdded(StoredItem const &item);
    void slotItemChanged(StoredItem const &item);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;

    void removeRead();
    void insertAndNotify(qint64 index, const StoredItem &item);
    void refreshMerge();
};

#endif // UNREADITEMMODEL_H
