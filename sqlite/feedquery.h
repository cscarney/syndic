/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SQLITE_FEEDQUERY_H
#define SQLITE_FEEDQUERY_H
#include <QDateTime>
#include <QSqlQuery>
#include <QUrl>
#include <QVariant>

namespace SqliteStorage
{
class FeedQuery : public QSqlQuery
{
public:
    FeedQuery(QSqlDatabase db, const QString &whereClause)
        : QSqlQuery(db)
    {
        prepare(
            "SELECT Feed.id, Feed.source, Feed.localId, Feed.displayName, Feed.category, Feed.url, Feed.link, Feed.icon, "
            "COUNT(Item.id), updateInterval, lastUpdate "
            "FROM Feed LEFT JOIN Item ON Item.feed=Feed.id AND Item.isRead=false "
            "WHERE "
            + whereClause + " GROUP BY Feed.id");
    }
    qint64 id() const
    {
        return value(0).toLongLong();
    }
    qint64 source() const
    {
        return value(1).toLongLong();
    }
    QString localId() const
    {
        return value(2).toString();
    }
    QString displayName() const
    {
        return value(3).toString();
    }
    QString category() const
    {
        return value(4).toString();
    }
    QUrl url() const
    {
        return value(5).toUrl();
    }
    QUrl link() const
    {
        return value(6).toUrl();
    }
    QUrl icon() const
    {
        return value(7).toUrl();
    }
    int unreadCount() const
    {
        return value(8).toInt();
    }
    qint64 updateInterval() const
    {
        return value(9).toLongLong();
    }
    QDateTime lastUpdate() const
    {
        return QDateTime::fromSecsSinceEpoch(value(10).toLongLong());
    }
};
}
#endif // SQLITE_FEEDQUERY_H
