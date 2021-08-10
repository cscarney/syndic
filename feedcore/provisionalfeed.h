#ifndef FEEDCORE_PROVISIONALFEED_H
#define FEEDCORE_PROVISIONALFEED_H
#include <Syndication/Feed>
#include "future.h"
#include "factory.h"
#include "updatablefeed.h"
namespace FeedCore {
/**
 * A minimal implementation of Feed, used for configuring and previewing feeds before commiting them to storage
 */
class ProvisionalFeed : public UpdatableFeed {
    Q_OBJECT
    Q_PROPERTY(Feed *targetFeed MEMBER m_targetFeed NOTIFY targetFeedChanged)
public:
    explicit ProvisionalFeed(QObject *parent=nullptr);

    /**
     * Retrieve preview articles.
     *
     * This will return an empty list until an update has successfully completed.
     */
    Future<ArticleRef> *getArticles(bool unreadFilter) final;

    void updateFromSource(const Syndication::FeedPtr &feed) final;

    /**
     * Copy this feed's configuration into the feed specified by the targetFeed property
     */
    Q_INVOKABLE void save();
signals:
    void targetFeedChanged();
private:
    Feed *m_targetFeed { nullptr };
    Syndication::FeedPtr m_feed;
    class ArticleImpl;
    SharedFactory<Syndication::ItemPtr, ArticleImpl> m_articles;
    void onUrlChanged();
};
}
#endif // FEEDCORE_PROVISIONALFEED_H
