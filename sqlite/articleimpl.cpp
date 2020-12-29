#include "sqlite/articleimpl.h"
#include <QSqlQuery>
#include <QVariant>
#include "sqlite/storageimpl.h"
#include "sqlite/feedimpl.h"

using namespace FeedCore;
using namespace Sqlite;

ArticleImpl::ArticleImpl(qint64 id, const QSharedPointer<FeedImpl> &feed):
    m_id{ id },
    m_feed{ feed }
{}

qint64 ArticleImpl::id() const
{
    return m_id;
}

// id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred
void ArticleImpl::updateFromQuery(const ItemQuery &q)
{
    populateTitle(q.headline());
    populateAuthor(q.author());
    populateDate(q.date());
    populateUrl(q.url());
    m_content = q.content();
    populateReadStatus(q.isRead());
}

void ArticleImpl::requestContent()
{
    emit gotContent(m_content);
}

void ArticleImpl::setRead(bool isRead)
{
    m_feed->setRead(this, isRead);
}
