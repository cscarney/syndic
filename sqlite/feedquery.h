#ifndef FEEDQUERY_H
#define FEEDQUERY_H
#include <QSqlQuery>
#include <QVariant>
#include <QUrl>

namespace Sqlite {
class FeedQuery : public QSqlQuery {
public:
    FeedQuery(QSqlDatabase db, const QString &whereClause) :
        QSqlQuery(db)
    {

        prepare("SELECT Feed.id, Feed.source, Feed.localId, Feed.displayName, Feed.url, COUNT(Item.id) "
                "FROM Feed LEFT JOIN Item ON Item.feed=Feed.id AND Item.isRead=false "
                "WHERE "+whereClause+" GROUP BY Feed.id");
    }
    inline qint64 id() const { return value(0).toLongLong(); }
    inline qint64 source() const { return value(1).toLongLong(); }
    inline QString localId() const { return value(2).toString(); }
    inline QString displayName() const { return value(3).toString(); }
    inline QUrl url() const { return value(4).toUrl(); }
    inline int unreadCount() const { return value(5).toInt(); }
};
}
#endif // FEEDQUERY_H
