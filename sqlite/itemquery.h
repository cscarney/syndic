/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QDateTime>
#include <QSqlQuery>
#include <QUrl>
#include <QVariant>

namespace SqliteStorage
{
class ItemQuery : public QSqlQuery
{
public:
    ItemQuery(const QSqlDatabase &db, const QString &whereClause)
        : QSqlQuery(db)
    {
        prepare(
            "SELECT id, feed, localId, headline, author, date, url, isRead, isStarred "
            "FROM Item WHERE "
            + whereClause);
    }

    qint64 id() const
    {
        return value(0).toLongLong();
    }
    qint64 feed() const
    {
        return value(1).toLongLong();
    }
    QString localId() const
    {
        return value(2).toString();
    }
    QString headline() const
    {
        return value(3).toString();
    }
    QString author() const
    {
        return value(4).toString();
    }
    QDateTime date() const
    {
        return QDateTime::fromSecsSinceEpoch(value(5).toLongLong());
    }
    QUrl url() const
    {
        return value(6).toUrl();
    }
    bool isRead() const
    {
        return value(7).toBool();
    }
    bool isStarred() const
    {
        return value(8).toBool();
    }
};
}
