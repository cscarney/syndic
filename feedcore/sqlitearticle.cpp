#include "sqlitearticle.h"

#include <QSqlQuery>
#include <QVariant>

#include "sqlitefeedstorage.h"
#include "sqlitefeed.h"

using namespace FeedCore;

SqliteArticle::SqliteArticle(const QSharedPointer<SqliteFeed> &feed, qint64 id):
    FeedCore::Article(FeedRef(feed)),
    m_id(id)
{

}

QSharedPointer<SqliteArticle> SqliteArticle::forId(const QSharedPointer<SqliteFeed> &feed, qint64 id)
{
    static QHash<qint64, QWeakPointer<SqliteArticle>> instances;
    auto &instance = instances[id];
    if (instance.isNull()) {
        auto newArticle = QSharedPointer<SqliteArticle>(new SqliteArticle(feed, id));
        instance = newArticle;
        return newArticle;
    }
    return instance.toStrongRef();
}

QSharedPointer<SqliteArticle> SqliteArticle::fromQuery(const QSharedPointer<SqliteFeed> &feed, const QSqlQuery &q)
{
    qint64 id = q.value(0).toLongLong();
    const auto &result = forId(feed, id);
    result->updateFromQuery(q);
    return result;
}

qint64 SqliteArticle::id() const
{
    return m_id;
}

SqliteFeed *SqliteArticle::feedImpl()
{
    return feed().objectCast<SqliteFeed>().get();
}

// id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred
void SqliteArticle::updateFromQuery(const QSqlQuery &q)
{
    populateTitle(q.value(3).toString());
    populateAuthor(q.value(4).toString());
    populateDate(QDateTime::fromSecsSinceEpoch(q.value(5).toLongLong()));
    populateUrl(q.value(6).toUrl());
    m_content = q.value(7).toString();
    populateReadStatus(q.value(8).toBool());
}

void SqliteArticle::requestContent()
{
    emit gotContent(m_content);
}

void SqliteArticle::setRead(bool isRead)
{
    feedImpl()->setItemRead(this, isRead);
}
