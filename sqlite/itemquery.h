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

    qint64 id() const { return value(0).toLongLong(); }
    qint64 feed() const { return value(1).toLongLong(); }
    QString localId() const { return value(2).toString(); }
    QString headline() const { return value(3).toString(); }
    QString author() const { return value(4).toString(); }
    QDateTime date() const { return QDateTime::fromSecsSinceEpoch(value(5).toLongLong()); }
    QUrl url() const { return value(6).toUrl(); }
    QString content() const { return value(7).toString(); }
    bool isRead() const { return value(8).toBool(); }
    bool isStarred() const { return value(9).toBool(); }
};
}
#endif // SQLITE_ITEMQUERY_H
