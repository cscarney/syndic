#ifndef ALLITEMSFEED_H
#define ALLITEMSFEED_H
#include "feed.h"

namespace FeedCore {
class Context;
class FeedRef;

class AllItemsFeed : public Feed
{
    Q_OBJECT
public:
    AllItemsFeed(Context *context, const QString &name, QObject *parent=nullptr);
    Future<ArticleRef> *startItemQuery(bool unreadFilter) final;
    Updater *updater() final;
private:
    Context *m_context;
    Updater *m_updater;
    void addFeed(const FeedRef &feed);
    void onFeedQueryFinished(Future<FeedRef> *sender);
    void onUnreadCountChanged(int delta);
    void onItemAdded(const ArticleRef &item);
};
}

#endif // ALLITEMSFEED_H
