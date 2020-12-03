#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H

#include <memory>

#include "managedlistmodel.h"
#include "enums.h"
#include "feedstorageoperation.h"
#include "storeditem.h"

class FeedManager;

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

    LoadStatus status();
    Q_PROPERTY(Enums::LoadStatus status READ status NOTIFY statusChanged);

    Q_INVOKABLE virtual void requestUpdate() {}
    Q_INVOKABLE void markAllRead();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private slots:
    void slotQueryFinished();
    void slotQueryFinishedMerge();
    void slotItemAdded(const StoredItem &item);
    void slotItemChanged(const StoredItem &item);
    void slotFeedStatusChanged(qint64 feedId, Enums::LoadStatus status);

signals:
    void unreadFilterChanged();
    void managerChanged();
    void statusChanged();

protected:
    virtual ItemQuery *startQuery() = 0;
    virtual bool itemFilter(const StoredItem &item) = 0;
    virtual void setStatusFromUpstream();
    void setStatus(LoadStatus status);
    void refresh();

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;

    void removeRead();
    void insertAndNotify(qint64 index, const StoredItem &item);
    void refreshMerge();
};

#endif // UNREADITEMMODEL_H
