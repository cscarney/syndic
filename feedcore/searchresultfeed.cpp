#include "searchresultfeed.h"

using namespace FeedCore;

SearchResultFeed::SearchResultFeed(QObject *parent)
    : FeedCore::Feed{parent}
{
}

QFuture<ArticleRef> SearchResultFeed::getArticles(bool unreadFilter)
{
    return m_context->searchArticles(m_query);
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
