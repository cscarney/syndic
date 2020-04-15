#ifndef BASICFEEDSTORAGE_H
#define BASICFEEDSTORAGE_H

#include <QObject>
#include "feedstorage.h"

class BasicFeedStorage : public FeedStorage
{
    Q_OBJECT
public:
    BasicFeedStorage(QObject *parent=nullptr);
    ~BasicFeedStorage();

    virtual FeedSource::Item getById(qint64 id)  override final;
    virtual qint64 storeItem(FeedSource::Item const &item) override final;
    virtual QList<FeedSource::Item> getAll() override final;

    virtual bool storeFeed(FeedSource::Feed) override final;
    virtual QList<FeedSource::Feed> getFeeds() override final;

    void save();
private:
    QHash<qint64,FeedSource::Item> m_itemsById;
    QList<FeedSource::Feed> m_feeds;

    int findFeed(qint64 id);
};

#endif // BASICFEEDSTORAGE_H
