#ifndef FEEDMANAGER_H
#define FEEDMANAGER_H

#include <memory>

#include <QObject>
#include <QAbstractItemModel>
#include <QUrl>
#include <Syndication/Feed>

class FeedUpdater;
class StoredItem;

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
    struct PrivData;
    std::unique_ptr<PrivData> priv;

private slots:
    void slotFeedLoaded(FeedUpdater *updater, Syndication::FeedPtr content);
};

#endif // FEEDMANAGER_H
