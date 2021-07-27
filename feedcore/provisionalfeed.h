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

/**
 * A minimal implementation of Feed, used for configuring and previewing feeds before commiting them to storage
 */
class ProvisionalFeed : public Feed {
    Q_OBJECT
    Q_PROPERTY(Feed *targetFeed MEMBER m_targetFeed NOTIFY targetFeedChanged)
public:
    explicit ProvisionalFeed(QObject *parent=nullptr);

    /**
     * An XMLUpdater that can be used to check the validity of a feed configuration and
     * retrieve a preview of the configured feed.
     */
    Updater *updater() final;

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
    XMLUpdater *m_updater { nullptr };
    Syndication::FeedPtr m_feed;
    SharedFactory<Syndication::ItemPtr, Preview::ArticleImpl> m_articles;
    void onUrlChanged();
};
}
#endif // PROVISIONALFEED_H
