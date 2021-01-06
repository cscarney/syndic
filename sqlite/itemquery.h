#ifndef SQLITE_ITEMQUERY_H
#define SQLITE_ITEMQUERY_H
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QUrl>

namespace Sqlite {
class ItemQuery : public QSqlQuery {
public:
    ItemQuery(QSqlDatabase db, const QString &whereClause) :
        QSqlQuery(db)
    {
        prepare("SELECT id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred "
                "FROM Item WHERE "+whereClause);
    }

    inline qint64 id() const { return value(0).toLongLong(); }
    inline qint64 feed() const { return value(1).toLongLong(); }
    inline QString localId() const { return value(2).toString(); }
    inline QString headline() const { return value(3).toString(); }
    inline QString author() const { return value(4).toString(); }
    inline QDateTime date() const { return QDateTime::fromSecsSinceEpoch(value(5).toLongLong()); }
    inline QUrl url() const { return value(6).toUrl(); }
    inline QString content() const { return value(7).toString(); }
    inline bool isRead() const { return value(8).toBool(); }
    inline bool isStarred() const { return value(9).toBool(); }
};
}
#endif // SQLITE_ITEMQUERY_H
