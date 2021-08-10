#ifndef FEEDCORE_STORAGE_H
#define FEEDCORE_STORAGE_H
#include <QObject>
#include <Syndication/Item>
#include <Syndication/Feed>
#include "future.h"

namespace FeedCore {
class Feed;

class Storage : public QObject {
    Q_OBJECT
public:
    explicit Storage(QObject *parent = nullptr) : QObject(parent) {};
    virtual Future<ArticleRef> *getAll() = 0;
    virtual Future<ArticleRef> *getUnread() = 0;
    virtual Future<ArticleRef> *getStarred() = 0;
    virtual Future<Feed *> *getFeeds() = 0;
    virtual Future<Feed *> *storeFeed(Feed *feed)=0;
};
}
#endif //FEEDCORE_STORAGE_H
