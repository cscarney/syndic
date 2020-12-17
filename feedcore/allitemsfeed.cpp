#include "allitemsfeed.h"

#include "context.h"
#include "articleref.h"

namespace FeedCore {

AllItemsFeed::AllItemsFeed(Context *context, const QString &name, QObject *parent) :
    Feed(parent),
    m_context(context)
{
    FeedQuery *q { context->startFeedQuery() };
    populateName(name);
    QObject::connect(q, &FeedStorageOperation::finished, this,
                     [this, q]{ onFeedQueryFinished(q); });
    QObject::connect(context, &Context::feedAdded, this, &AllItemsFeed::addFeed);
}

ItemQuery *AllItemsFeed::startItemQuery(bool unreadFilter)
{
    return m_context->startQuery(unreadFilter);
}

void AllItemsFeed::addFeed(const FeedRef &feed)
{
    incrementUnreadCount(feed->unreadCount());
    QObject::connect(feed.get(), &Feed::itemAdded, this, &AllItemsFeed::onItemAdded);
    QObject::connect(feed.get(), &Feed::unreadCountChanged, this, &AllItemsFeed::incrementUnreadCount);
}

void AllItemsFeed::onFeedQueryFinished(FeedQuery *sender)
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
