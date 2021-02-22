#ifndef SQLITE_FEEDQUERY_H
#define SQLITE_FEEDQUERY_H
#include <QSqlQuery>
#include <QDateTime>
#include <QVariant>
#include <QUrl>

namespace Sqlite {
class FeedQuery : public QSqlQuery {
public:
    FeedQuery(QSqlDatabase db, const QString &whereClause) :
        QSqlQuery(db)
    {

        prepare("SELECT Feed.id, Feed.source, Feed.localId, Feed.displayName, Feed.url, COUNT(Item.id), updateInterval, lastUpdate "
                "FROM Feed LEFT JOIN Item ON Item.feed=Feed.id AND Item.isRead=false "
                "WHERE "+whereClause+" GROUP BY Feed.id ORDER BY Feed.displayName ASC");
    }
    qint64 id() const { return value(0).toLongLong(); }
    qint64 source() const { return value(1).toLongLong(); }
    QString localId() const { return value(2).toString(); }
    QString displayName() const { return value(3).toString(); }
    QUrl url() const { return value(4).toUrl(); }
    int unreadCount() const { return value(5).toInt(); }
    qint64 updateInterval() const { return value(6).toLongLong(); }
    QDateTime lastUpdate() const { return QDateTime::fromSecsSinceEpoch(value(7).toLongLong()); }
};
}
#endif // SQLITE_FEEDQUERY_H
