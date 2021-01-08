#include "allitemsfeed.h"
#include "context.h"
#include "feedref.h"
#include "updater.h"
#include "article.h"
namespace FeedCore {

namespace {
    class AllUpdater : public FeedCore::Updater {
    public:
        AllUpdater(AllItemsFeed *feed, Context *context, QObject *parent):
            FeedCore::Updater(feed, 0, 0, parent),
            m_context { context }
        {}

        void run() final {
            m_context->requestUpdate();
            finish();
        }

    private:
        Context *m_context;
    };
}

AllItemsFeed::AllItemsFeed(Context *context, const QString &name, QObject *parent) :
    Feed(parent),
    m_context{ context },
    m_updater{ new AllUpdater(this, context, this) }
{
    Future<FeedRef> *q { context->getFeeds() };
    setName(name);
    QObject::connect(q, &BaseFuture::finished, this,
                     [this, q]{ onGetFeedsFinished(q); });
    QObject::connect(context, &Context::feedAdded, this, &AllItemsFeed::addFeed);
}

Future<ArticleRef> *AllItemsFeed::getArticles(bool unreadFilter)
{
    return m_context->getArticles(unreadFilter);
}

Updater *AllItemsFeed::updater()
{
    return m_updater;
}

void AllItemsFeed::addFeed(const FeedRef &feed)
{
    m_feeds.insert(feed);
    Feed *fp = feed.get();
    incrementUnreadCount(fp->unreadCount());
    syncFeedStatus(fp);
    QObject::connect(fp, &Feed::articleAdded, this, &AllItemsFeed::onArticleAdded);
    QObject::connect(fp, &Feed::unreadCountChanged, this, &AllItemsFeed::incrementUnreadCount);
    QObject::connect(fp, &Feed::statusChanged, this, [this, fp]{ syncFeedStatus(fp); });
}

void AllItemsFeed::onGetFeedsFinished(Future<FeedRef> *sender)
{
    for (const auto &feed : sender->result()){
        addFeed(feed);
    }
}

void AllItemsFeed::onArticleAdded(const ArticleRef &article)
{
    emit articleAdded(article);
}

void AllItemsFeed::syncFeedStatus(Feed *sender)
{
    if (sender->status() == LoadStatus::Updating) {
        m_active.insert(sender);
    } else {
        m_active.remove(sender);
    }
    setStatus(m_active.isEmpty() ? LoadStatus::Idle : LoadStatus::Updating);
}

}
