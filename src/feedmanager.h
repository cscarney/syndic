#ifndef FEEDMANAGER_H
#define FEEDMANAGER_H

#include <memory>

#include <QObject>
#include <QAbstractItemModel>
#include <QUrl>
#include <Syndication/Feed>

#include "feedstorageoperation.h"

class FeedUpdater;
class ItemModel;

class FeedManager : public QObject
{
    Q_OBJECT
public:
    explicit FeedManager(QObject *parent = nullptr);
    ~FeedManager();

    /* Returns a model owned by the FeedManager (parent=this) */
    Q_INVOKABLE QAbstractItemModel *getFeedListModel();

    Q_INVOKABLE void setRead(qint64 id, bool value=true);
    Q_INVOKABLE void setStarred(qint64 id, bool value=true);
    Q_INVOKABLE void setAllRead(QVariant const &feedFilter=QVariant(), bool value=false);
    Q_INVOKABLE void addFeed(QUrl url);

    ItemQuery *startQuery(std::optional<qint64> feedFilter, bool unreadFilter);

signals:
    void itemAdded(StoredItem const &item);
    void itemChanged(StoredItem const &item);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;

private slots:
    void slotFeedLoaded(FeedUpdater *updater, Syndication::FeedPtr content);
};

#endif // FEEDMANAGER_H
