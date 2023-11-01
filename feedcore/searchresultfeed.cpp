#include "searchresultfeed.h"

using namespace FeedCore;

class SearchResultFeed::Updater : public Feed::Updater
{
public:
    Updater(SearchResultFeed *feed, QObject *parent)
        : Feed::Updater(feed, parent)
    {
    }

private:
    void run() override
    {
        QMetaObject::invokeMethod(this, &Updater::finish, Qt::QueuedConnection);
    }
};

SearchResultFeed::SearchResultFeed(QObject *parent)
    : FeedCore::Feed{parent}
    , m_updater{new Updater(this, this)}
{
}

QFuture<ArticleRef> SearchResultFeed::getArticles(bool unreadFilter)
{
    return m_context->searchArticles(m_query);
}

Feed::Updater *SearchResultFeed::updater()
{
    return m_updater;
}

Context *SearchResultFeed::context() const
{
    return m_context;
}

void SearchResultFeed::setContext(Context *newContext)
{
    if (m_context == newContext)
        return;
    m_context = newContext;
    emit contextChanged();
    emit reset();
}

QString SearchResultFeed::query() const
{
    return m_query;
}

void SearchResultFeed::setQuery(const QString &newQuery)
{
    if (m_query == newQuery)
        return;
    m_query = newQuery;
    emit queryChanged();
    emit reset();
}
