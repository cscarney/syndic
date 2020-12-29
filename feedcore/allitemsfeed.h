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
    Future<ArticleRef> *getArticles(bool unreadFilter) final;
    Updater *updater() final;
private:
    Context *m_context { nullptr };
    Updater *m_updater { nullptr };
    void addFeed(const FeedRef &feed);
    void onGetFeedsFinished(Future<FeedRef> *sender);
    void onUnreadCountChanged(int delta);
    void onArticleAdded(const ArticleRef &article);
};
}

#endif // ALLITEMSFEED_H
