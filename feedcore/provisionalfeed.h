#ifndef PROVISIONALFEED_H
#define PROVISIONALFEED_H
#include <Syndication/Feed>
#include "future.h"
#include "uniquefactory.h"
#include "feed.h"
namespace FeedCore {
class XMLUpdater;
namespace Preview {
    class ArticleImpl;
}

class ProvisionalFeed : public Feed {
    Q_OBJECT
public:
    explicit ProvisionalFeed(QObject *parent=nullptr);
    Updater *updater() final;
    Future<ArticleRef> *getArticles(bool unreadFilter) final;
    void updateFromSource(const Syndication::FeedPtr &feed) final;
private:
    XMLUpdater *m_updater { nullptr };
    Syndication::FeedPtr m_feed;
    UniqueFactory<Syndication::ItemPtr, Preview::ArticleImpl> m_articles;
    void onUrlChanged();
};
}
#endif // PROVISIONALFEED_H
