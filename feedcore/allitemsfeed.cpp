#include "allitemsfeed.h"
#include "context.h"
#include "feedref.h"
#include "updater.h"

namespace FeedCore {

namespace {
    class AllUpdater : public FeedCore::Updater {
    public:
        AllUpdater(AllItemsFeed *feed, Context *context, QObject *parent):
            FeedCore::Updater(feed, 0, 0, parent),
            m_context(context)
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
    m_context(context),
    m_updater(new AllUpdater(this, context, this))
{
    Future<FeedRef> *q { context->startFeedQuery() };
    populateName(name);
    QObject::connect(q, &BaseFuture::finished, this,
                     [this, q]{ onFeedQueryFinished(q); });
    QObject::connect(context, &Context::feedAdded, this, &AllItemsFeed::addFeed);
}

Future<ArticleRef> *AllItemsFeed::startItemQuery(bool unreadFilter)
{
    return m_context->startQuery(unreadFilter);
}

Updater *AllItemsFeed::updater()
{
    return m_updater;
}

void AllItemsFeed::addFeed(const FeedRef &feed)
{
    incrementUnreadCount(feed->unreadCount());
    QObject::connect(feed.get(), &Feed::itemAdded, this, &AllItemsFeed::onItemAdded);
    QObject::connect(feed.get(), &Feed::unreadCountChanged, this, &AllItemsFeed::incrementUnreadCount);
}

void AllItemsFeed::onFeedQueryFinished(Future<FeedRef> *sender)
{
    for (const auto &feed : sender->result()){
        addFeed(feed);
    }
}

void AllItemsFeed::onItemAdded(const ArticleRef &item)
{
    emit itemAdded(item);
}

}
