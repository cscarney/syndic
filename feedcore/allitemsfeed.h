#ifndef ALLITEMSFEED_H
#define ALLITEMSFEED_H
#include <QSet>
#include "feed.h"

namespace FeedCore {
class Context;

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
    QSet<Feed *> m_active;
    void addFeed(Feed *feed);
    void onGetFeedsFinished(Future<Feed*> *sender);
    void onUnreadCountChanged(int delta);
    void onArticleAdded(const ArticleRef &article);
    void syncFeedStatus(FeedCore::Feed *sender);
    void onFeedDestroyed(FeedCore::Feed *sender);
};
}

#endif // ALLITEMSFEED_H
