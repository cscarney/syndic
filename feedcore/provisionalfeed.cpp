#include "provisionalfeed.h"
#include "xmlupdater.h"
#include "preview/articleimpl.h"
using namespace FeedCore;

void ProvisionalFeed::onUrlChanged() {
    updater()->abort();
    m_feed = nullptr;
    emit reset();
}

ProvisionalFeed::ProvisionalFeed(QObject *parent) :
    Feed(parent),
    m_updater(new XMLUpdater(this, this))
{
    QObject::connect(this, &Feed::urlChanged, m_updater, &Updater::abort);
}

FeedCore::Updater *ProvisionalFeed::updater()
{
    return m_updater;
}

Future<ArticleRef> *ProvisionalFeed::getArticles(bool unreadFilter)
{
    return Future<ArticleRef>::yield(this, [this](auto *op){
        if (!m_feed) {
            return;
        }
        for(const auto &item : m_feed->items()) {
            op->appendResult(m_articles.getInstance(item));
        }
    });
}

void ProvisionalFeed::updateFromSource(const Syndication::FeedPtr &feed)
{
    setName(feed->title());
    setUnreadCount(feed->items().size());
    m_feed = feed;
    emit reset();
}
