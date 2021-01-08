#include "preview/articleimpl.h"
#include <Syndication/Person>
using namespace FeedCore::Preview;

void ArticleImpl::requestContent()
{
    QString content { m_item->content() };
    emit gotContent(content.isEmpty() ? content : m_item->description() );
}

FeedCore::Preview::ArticleImpl::ArticleImpl(Syndication::ItemPtr item, QObject *parent):
    Article(parent),
    m_item(item)
{
    setTitle(item->title());
    setUrl(item->link());
    setDate(QDateTime::fromTime_t(item->dateUpdated()));
    auto authors = item->authors();
    setAuthor(authors.isEmpty() ? "" : authors[0]->name());
}
