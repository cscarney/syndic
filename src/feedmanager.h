#ifndef FEEDMANAGER_H
#define FEEDMANAGER_H

#include <memory>

#include <QObject>
#include <QAbstractItemModel>
#include <QUrl>
#include <Syndication/Feed>

#include "enums.h"
#include "feedstorageoperation.h"

class FeedUpdater;
class ItemModel;

class FeedManager : public QObject
{
    Q_OBJECT
public:
    explicit FeedManager(QObject *parent = nullptr);
    ~FeedManager();

    FeedQuery *startFeedQuery();

    Q_INVOKABLE void setRead(qint64 id, bool value=true);
    Q_INVOKABLE void setStarred(qint64 id, bool value=true);
    Q_INVOKABLE void addFeed(QUrl url);

    ItemQuery *startQuery(std::optional<qint64> feedFilter, bool unreadFilter);
    LoadStatus getFeedStatus(qint64 feedId);
    qint64 getFeedUnreadCount(qint64 feedId);

    void requestUpdate();
    void requestUpdate(qint64 feedId);
    bool updatesInProgress();

signals:
    void itemAdded(StoredItem const &item);
    void itemChanged(StoredItem const &item);
    void itemReadChanged(StoredItem const &item);
    void itemStarredChanged(StoredItem const &item);

    void feedAdded(StoredFeed const &feed);
    void feedStatusChanged(qint64 feedId, LoadStatus status);
    void feedNameChanged(qint64 feedId, QString newName);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;

private slots:
    void slotFeedLoaded(FeedUpdater *updater, Syndication::FeedPtr content);
    void slotFeedStatusChanged(FeedUpdater *updater, LoadStatus status);
};

#endif // FEEDMANAGER_H
