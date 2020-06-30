#ifndef FEEDMANAGER_H
#define FEEDMANAGER_H

#include <memory>

#include <QObject>
#include <QAbstractItemModel>
#include <QUrl>

#include "feedsource.h"
#include "storeditem.h"

class FeedStorage;
class FeedListModel;

class FeedManager : public QObject
{
    Q_OBJECT
public:
    explicit FeedManager(QObject *parent = nullptr);
    ~FeedManager();

    /* Returns a model owned by the caller (parent=null) */
    QAbstractItemModel *getModel(std::optional<qint64> feedFilter=std::nullopt, bool unreadOnly=false);
    Q_INVOKABLE QAbstractItemModel *getModel(QVariant const& feedFilter, bool unreadOnly=false);

    /* Returns a model owned by the FeedManager (parent=this) */
    Q_INVOKABLE QAbstractItemModel *getFeedListModel();

    Q_INVOKABLE void setRead(qint64 id, bool value=true);
    Q_INVOKABLE void setStarred(qint64 id, bool value=true);
    Q_INVOKABLE void setAllRead(QVariant const &feedFilter=QVariant(), bool value=false);
    Q_INVOKABLE void addFeed(QUrl url);

signals:
    void itemAdded(StoredItem const &item);
    void itemChanged(StoredItem const &item);

private:
    std::unique_ptr<FeedSource> m_source;
    std::unique_ptr<FeedStorage> m_storage;
    FeedListModel *m_feedList;

    void receiveItem(FeedSource::Item const &item);
    void receiveFeed(FeedSource::Feed const &feed);
};

#endif // FEEDMANAGER_H
