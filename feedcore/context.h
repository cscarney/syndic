#ifndef FEEDMANAGER_H
#define FEEDMANAGER_H

#include <memory>

#include <QObject>
#include <QUrl>
#include <Syndication/Feed>

#include "enums.h"
#include "feedref.h"
#include "feedstorageoperation.h"

namespace FeedCore {

class FeedUpdater;

class Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject *parent = nullptr);
    ~Context();

    FeedQuery *startFeedQuery();

    Q_INVOKABLE void setRead(qint64 id, bool value=true);
    Q_INVOKABLE void setStarred(qint64 id, bool value=true);
    Q_INVOKABLE void addFeed(const QUrl &url);

    ItemQuery *startQuery(const FeedRef &feedFilter, bool unreadFilter);
    LoadStatus getFeedStatus(const FeedRef &feed);
    qint64 getFeedUnreadCount(const FeedRef &feed);

    void requestUpdate();
    void requestUpdate(const FeedRef &feed);
    bool updatesInProgress();

signals:
    void itemAdded(const FeedCore::StoredItem &item);
    void itemChanged(const FeedCore::StoredItem &item);
    void itemReadChanged(const FeedCore::StoredItem &item);
    void itemStarredChanged(const FeedCore::StoredItem &item);

    void feedAdded(const FeedCore::FeedRef &feed);
    void feedStatusChanged(const FeedCore::FeedRef &feedId, FeedCore::LoadStatus status);
    void feedNameChanged(const FeedCore::FeedRef &feedId, const QString &newName);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;

private slots:
    void slotFeedLoaded(FeedCore::FeedUpdater *updater, const Syndication::FeedPtr &content);
    void slotFeedStatusChanged(FeedCore::FeedUpdater *updater, FeedCore::LoadStatus status);
};

}

#endif // FEEDMANAGER_H
