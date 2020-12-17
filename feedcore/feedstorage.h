#ifndef FEEDSTORAGE_H
#define FEEDSTORAGE_H

#include <QObject>
#include <Syndication/Item>
#include <Syndication/Feed>

#include "feedstorageoperation.h"

namespace FeedCore {

class FeedStorage : public QObject {
    Q_OBJECT
public:
    explicit FeedStorage(QObject *parent = nullptr) : QObject(parent) {};

    virtual ItemQuery *getAll() = 0;
    virtual ItemQuery *getUnread() = 0;

    virtual FeedQuery *getFeeds() = 0;
    virtual FeedQuery *storeFeed(const QUrl &url)=0;
    virtual FeedQuery *updateFeed(FeedRef &storedFeed, const Syndication::FeedPtr &update)=0;
};

}

#endif // FEEDSTORAGE_H
