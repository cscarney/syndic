#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H

#include <memory>

#include <QAbstractListModel>
#include <QQmlParserStatus>

#include "enums.h"
#include "feedstorageoperation.h"
#include "storeditem.h"

class FeedManager;

class ItemModel : public QAbstractListModel, public QQmlParserStatus
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

    bool unreadFilter() const;
    void setUnreadFilter(bool unreadFilter);
    Q_PROPERTY(bool unreadFilter READ unreadFilter WRITE setUnreadFilter NOTIFY unreadFilterChanged);

    FeedManager *manager() const;
    void setManager(FeedManager *manager);
    Q_PROPERTY(FeedManager *manager READ manager WRITE setManager NOTIFY managerChanged);

    LoadStatus status();
    Q_PROPERTY(Enums::LoadStatus status READ status NOTIFY statusChanged);

    void refresh();
    Q_INVOKABLE virtual void requestUpdate() {}
    Q_INVOKABLE void markAllRead();
    void classBegin() override;
    void componentComplete() override;

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

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;

    void removeRead();
    void insertAndNotify(qint64 index, const StoredItem &item);
    void refreshMerge();
};

#endif // UNREADITEMMODEL_H
