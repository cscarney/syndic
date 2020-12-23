#ifndef ALLITEMSFEED_H
#define ALLITEMSFEED_H

#include "feed.h"
#include "feedref.h"

namespace FeedCore {

class Context;

class AllItemsFeed : public Feed
{
    Q_OBJECT
public:
    AllItemsFeed(Context *context, const QString &name, QObject *parent=nullptr);
    ItemQuery *startItemQuery(bool unreadFilter) final;

private:
    Context *m_context;
    void addFeed(const FeedCore::FeedRef &feed);
    void onFeedQueryFinished(FeedQuery *sender);
    void onUnreadCountChanged(int delta);
    void onItemAdded(const FeedCore::ArticleRef &item);
};

}

#endif // ALLITEMSFEED_H
