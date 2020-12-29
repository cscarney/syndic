#include "sqlite/articleimpl.h"
#include <QSqlQuery>
#include <QVariant>
#include "sqlite/storageimpl.h"
#include "sqlite/feedimpl.h"

using namespace FeedCore;
using namespace Sqlite;

ArticleImpl::ArticleImpl(const QSharedPointer<FeedImpl> &feed, qint64 id):
    FeedCore::Article(FeedRef(feed)),
    m_id(id)
{

}

QSharedPointer<ArticleImpl> ArticleImpl::forId(const QSharedPointer<FeedImpl> &feed, qint64 id)
{
    static QHash<qint64, QWeakPointer<ArticleImpl>> instances;
    auto &instance = instances[id];
    if (instance.isNull()) {
        auto newArticle = QSharedPointer<ArticleImpl>(new ArticleImpl(feed, id));
        instance = newArticle;
        return newArticle;
    }
    return instance.toStrongRef();
}

QSharedPointer<ArticleImpl> ArticleImpl::fromQuery(const QSharedPointer<FeedImpl> &feed, const QSqlQuery &q)
{
    qint64 id = q.value(0).toLongLong();
    const auto &result = forId(feed, id);
    result->updateFromQuery(q);
    return result;
}

qint64 ArticleImpl::id() const
{
    return m_id;
}

FeedImpl *ArticleImpl::feedImpl()
{
    return feed().objectCast<FeedImpl>().get();
}

// id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred
void ArticleImpl::updateFromQuery(const QSqlQuery &q)
{
    populateTitle(q.value(3).toString());
    populateAuthor(q.value(4).toString());
    populateDate(QDateTime::fromSecsSinceEpoch(q.value(5).toLongLong()));
    populateUrl(q.value(6).toUrl());
    m_content = q.value(7).toString();
    populateReadStatus(q.value(8).toBool());
}

void ArticleImpl::requestContent()
{
    emit gotContent(m_content);
}

void ArticleImpl::setRead(bool isRead)
{
    feedImpl()->setItemRead(this, isRead);
}
