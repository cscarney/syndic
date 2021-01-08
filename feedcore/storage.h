#ifndef STORAGE_H
#define STORAGE_H
#include <QObject>
#include <Syndication/Item>
#include <Syndication/Feed>
#include "future.h"
#include "feedref.h"

namespace FeedCore {
class ProvisionalFeed;
class Storage : public QObject {
    Q_OBJECT
public:
    explicit Storage(QObject *parent = nullptr) : QObject(parent) {};
    virtual Future<ArticleRef> *getAll() = 0;
    virtual Future<ArticleRef> *getUnread() = 0;
    virtual Future<FeedRef> *getFeeds() = 0;
    virtual Future<FeedRef> *storeFeed(ProvisionalFeed *feed)=0;
};
}
#endif //STORAGE_H
