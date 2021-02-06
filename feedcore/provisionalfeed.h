#ifndef PROVISIONALFEED_H
#define PROVISIONALFEED_H
#include <Syndication/Feed>
#include "future.h"
#include "factory.h"
#include "feed.h"
namespace FeedCore {
class XMLUpdater;
namespace Preview {
    class ArticleImpl;
}

class ProvisionalFeed : public Feed {
    Q_OBJECT
    Q_PROPERTY(Feed *targetFeed MEMBER m_targetFeed NOTIFY targetFeedChanged)
public:
    explicit ProvisionalFeed(QObject *parent=nullptr);
    Updater *updater() final;
    Future<ArticleRef> *getArticles(bool unreadFilter) final;
    void updateFromSource(const Syndication::FeedPtr &feed) final;
    Q_INVOKABLE void save();
signals:
    void targetFeedChanged();
private:
    Feed *m_targetFeed { nullptr };
    XMLUpdater *m_updater { nullptr };
    Syndication::FeedPtr m_feed;
    SharedFactory<Syndication::ItemPtr, Preview::ArticleImpl> m_articles;
    void onUrlChanged();
};
}
#endif // PROVISIONALFEED_H
