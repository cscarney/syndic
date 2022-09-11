/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feeddatabase.h"
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace SqliteStorage
{
static const QString db_name_fmt = QStringLiteral("Sqlite_FeedDatabase_%1");

QSqlDatabase FeedDatabase::db()
{
    return QSqlDatabase::database(m_dbName);
}

static int getVersion(QSqlDatabase &db)
{
    QSqlQuery q("PRAGMA user_version", db);
    if (!q.next()) {
        qCritical() << "getVersion returned empty set";
        return 0;
    }

    return q.value(0).toInt();
}

static bool exec(QSqlDatabase &db, const QString &queryString)
{
    QSqlQuery q(db);
    if (!q.exec(queryString)) {
        qWarning() << "SQL Error: " + q.lastError().text();
        return false;
    }
    return true;
}

static bool exec(QSqlDatabase &db, const QVector<QString> &queries)
{
    for (const auto &q : queries) {
        if (!exec(db, q)) {
            return false;
        }
    }
    return true;
}

static void initDatabase(QSqlDatabase &db)
{
    const auto &v = getVersion(db);
    bool success = true;
    switch (v) {
    case 0:
        success = exec(db,
                       {"CREATE TABLE Feed("
                        "id INTEGER PRIMARY KEY,"
                        "displayName TEXT,"
                        "category TEXT,"
                        "url TEXT NOT NULL,"
                        "link TEXT,"
                        "icon TEXT,"
                        "updateInterval INTEGER,"
                        "lastUpdate INTEGER,"
                        "expireAge INTEGER);",

                        "CREATE TABLE Item("
                        "id INTEGER PRIMARY KEY,"
                        "feed INTEGER REFERENCES Feed,"
                        "localId TEXT NOT NULL,"
                        "headline TEXT,"
                        "author TEXT,"
                        "date INTEGER,"
                        "url TEXT,"
                        "feedContent TEXT,"
                        "isRead INTEGER,"
                        "isStarred INTEGER,"
                        "UNIQUE(feed,localId));",

                        "PRAGMA user_version = 1;"});
        // fall through

    case 1:
        success = success
            && exec(db,
                    {"ALTER TABLE Item "
                     "ADD COLUMN readableContent TEXT;",

                     "ALTER TABLE Feed "
                     "ADD COLUMN flags INTEGER;",

                     "PRAGMA user_version = 2;"});
        break;

    case 2:
        break;

    default:
        // database is newer than we support
        success = false;
    }
    if (!success) {
        qCritical() << "Database initialization failed!";
        db.close();
    }
}

FeedDatabase::FeedDatabase(const QString &filePath)
{
    static int dbCount{0};
    m_dbName = db_name_fmt.arg(++dbCount);
    auto db = QSqlDatabase::addDatabase("QSQLITE", m_dbName);
    db.setDatabaseName(filePath);
    if (!db.open()) {
        qCritical() << "Failed to open database!";
    } else {
        initDatabase(db);
    }
}

FeedDatabase::~FeedDatabase()
{
    db().close();
}

static const QString select_sort = QStringLiteral("ORDER BY date DESC");

ItemQuery FeedDatabase::selectAllItems()
{
    ItemQuery q(db(), "1 " + select_sort);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectAllItems: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectUnreadItems()
{
    ItemQuery q(db(), "isRead=0 " + select_sort);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectUnreadItems: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectStarredItems()
{
    ItemQuery q(db(), "isStarred=1 " + select_sort);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectStarredItems: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItemsByFeed(qint64 feedId)
{
    ItemQuery q(db(), "feed=:feed " + select_sort);
    q.bindValue(":feed", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItemsByFeed: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectUnreadItemsByFeed(qint64 feedId)
{
    ItemQuery q(db(), "feed=:feed AND isRead=0 " + select_sort);
    q.bindValue(":feed", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectUnreadItemsByFeed: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItem(qint64 id)
{
    ItemQuery q(db(), "id=:id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItem: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItem(qint64 feed, const QString &localId)
{
    ItemQuery q(db(), "feed=:feed AND localId=:localId");
    q.bindValue(":feed", feed);
    q.bindValue(":localId", localId);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItem: " + q.lastError().text();
    }
    return q;
}

QString FeedDatabase::selectItemContent(qint64 id)
{
    QSqlQuery q(db());
    q.prepare("SELECT feedContent FROM Item WHERE id=:id LIMIT 1");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItemContent: " << q.lastError().text();
        return QString();
    }
    if (!q.next()) {
        return QString();
    }
    return q.value(0).toString();
}

QString FeedDatabase::selectItemReadableContent(qint64 id)
{
    QSqlQuery q(db());
    q.prepare("SELECT readableContent FROM Item WHERE id=:id AND readableContent IS NOT NULL LIMIT 1");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItemReadableContent: " << q.lastError().text();
        return QString();
    }
    if (!q.next()) {
        return QString();
    }
    return q.value(0).toString();
}

std::optional<qint64> FeedDatabase::selectItemId(qint64 feedId, const QString &localId)
{
    QSqlQuery q(db());
    q.prepare(
        "SELECT id FROM Item "
        "WHERE feed=:feed AND localId=:localId;");
    q.bindValue(":feed", feedId);
    q.bindValue(":localId", localId);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItemId: " + q.lastError().text();
        return std::nullopt;
    }
    if (!q.next()) {
        return std::nullopt;
    }
    return q.value(0).toLongLong();
}

std::optional<qint64> FeedDatabase::insertItem(qint64 feedId,
                                               const QString &localId,
                                               const QString &title,
                                               const QString &author,
                                               time_t date,
                                               const QUrl &url,
                                               const QString &content)
{
    QSqlQuery q(db());
    q.prepare(
        "INSERT INTO Item (id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred) "
        "VALUES (:id, :feed, :localId, :headline, :author, :date, :url, :feedContent, :isRead, :isStarred);");
    q.bindValue(":feed", feedId);
    q.bindValue(":localId", localId);
    q.bindValue(":headline", title);
    q.bindValue(":author", author);
    q.bindValue(":date", qint64(date));
    q.bindValue(":url", url.toString());
    q.bindValue(":feedContent", content);
    q.bindValue(":isRead", false);
    q.bindValue(":isStarred", false);
    if (!q.exec()) {
        qWarning() << "SQL Error in insertItem: " + q.lastError().text();
        return std::nullopt;
    }
    return q.lastInsertId().toLongLong();
}

void FeedDatabase::updateItemHeaders(qint64 id, const QString &title, const QString &author, const QUrl &url)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "headline=:headline,"
        "author=:author,"
        "url=:url "
        "WHERE id=:id;");
    q.bindValue(":headline", title);
    q.bindValue(":author", author);
    q.bindValue(":url", url.toString());
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateItemHeaders: " + q.lastError().text();
    }
}

void FeedDatabase::updateItemDate(qint64 id, time_t date)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "date=:date "
        "WHERE id=:id;");
    q.bindValue(":date", qint64(date));
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateItemDate: " + q.lastError().text();
    }
}

void FeedDatabase::updateItemContent(qint64 id, const QString &content)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "feedContent=:feedContent "
        "WHERE id=:id;");
    q.bindValue(":feedContent", content);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateItemContent: " + q.lastError().text();
    }
}

void FeedDatabase::updateItemReadableContent(qint64 id, const QString &readableContent)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "readableContent=:readableContent "
        "WHERE id=:id;");
    q.bindValue(":readableContent", readableContent);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateItemReadableContent: " + q.lastError().text();
    }
}

void FeedDatabase::updateItemRead(qint64 id, bool isRead)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "isRead=:isRead "
        "WHERE id=:id;");
    q.bindValue(":isRead", isRead);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateItemRead: " + q.lastError().text();
    }
}

void FeedDatabase::updateItemStarred(qint64 id, bool isStarred)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "isStarred=:isStarred "
        "WHERE id=:id;");
    q.bindValue(":isStarred", isStarred);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateItemStarred: " + q.lastError().text();
    }
}

void FeedDatabase::deleteItemsForFeed(qint64 feedId)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM Item WHERE feed=:feed");
    q.bindValue(":feed", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in deleteItemsForFeed: " << q.lastError().text();
    }
}

void FeedDatabase::deleteItemsOlderThan(qint64 feedId, const QDateTime &olderThan)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM Item WHERE feed=:feed AND isStarred!=1 AND date<:olderThan");
    q.bindValue(":feed", feedId);
    q.bindValue(":olderThan", olderThan.toSecsSinceEpoch());
    if (!q.exec()) {
        qWarning() << "SQL Error in deleteItemsForFeed: " << q.lastError().text();
    }
}

FeedQuery FeedDatabase::selectAllFeeds()
{
    FeedQuery q(db(), "1");
    if (!q.exec()) {
        qWarning() << "SQL Error: " + q.lastError().text();
    }
    return q;
}

FeedQuery FeedDatabase::selectFeed(qint64 feedId)
{
    FeedQuery q(db(), "Feed.id=:id");
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectFeed: " + q.lastError().text();
    }
    return q;
}

std::optional<qint64> FeedDatabase::insertFeed(const QUrl &url)
{
    QSqlQuery q(db());
    const QString &urlString = url.toString();
    const QString &urlHost = url.host();
    q.prepare(
        "INSERT INTO Feed (displayName, url) "
        "VALUES (:displayName, :url);");
    q.bindValue(":displayName", urlHost);
    q.bindValue(":url", urlString);
    if (!q.exec()) {
        qWarning() << "SQL Error in insertFeed: " + q.lastError().text();
        return std::nullopt;
    }
    return q.lastInsertId().toLongLong();
}

void FeedDatabase::updateFeedName(qint64 feedId, const QString &newName)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "displayName=:displayName "
        "WHERE id=:id");
    q.bindValue(":displayName", newName);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeedName: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedUrl(qint64 feedId, const QUrl &url)
{
    QSqlQuery q(db());
    const QString &urlString = url.toString();
    q.prepare(
        "UPDATE Feed SET "
        "url=:url "
        "WHERE id=:id");
    q.bindValue(":url", urlString);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeedUrl: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedCategory(qint64 feedId, const QString &category)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "category=:category "
        "WHERE id=:id");
    q.bindValue(":category", category);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeedName: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedLink(qint64 feedId, const QString &link)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "link=:link "
        "WHERE id=:id");
    q.bindValue(":link", link);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeedLink: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedIcon(qint64 feedId, const QString &icon)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "icon=:icon "
        "WHERE id=:id");
    q.bindValue(":icon", icon);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeedIcon: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedUpdateInterval(qint64 feedId, qint64 updateInterval)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "updateInterval=:updateInterval "
        "WHERE id=:id");
    q.bindValue(":updateInterval", updateInterval);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeed: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedLastUpdate(qint64 feedId, const QDateTime &lastUpdate)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "lastUpdate=:lastUpdate "
        "WHERE id=:id");
    if (lastUpdate.isValid()) {
        q.bindValue(":lastUpdate", lastUpdate.toSecsSinceEpoch());
    } else {
        q.bindValue(":lastUpdate", QVariant(QVariant::LongLong));
    }
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeed: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedExpireAge(qint64 feedId, qint64 expireAge)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "expireAge=:expireAge "
        "WHERE id=:id");
    q.bindValue(":expireAge", expireAge);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeedExpireAge: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedFlags(qint64 feedId, int flags)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Feed SET "
        "flags=:flags "
        "WHERE id=:id");
    q.bindValue(":flags", flags);
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateFeedFlags: " << q.lastError().text();
    }
}

void FeedDatabase::deleteFeed(qint64 feedId)
{
    QSqlQuery q(db());
    q.prepare("DELETE FROM Feed WHERE id=:id");
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qWarning() << "SQL Error in deleteFeed: " << q.lastError().text();
    }
}

void FeedDatabase::beginTransaction()
{
    db().exec("BEGIN TRANSACTION");
}

void FeedDatabase::commitTransaction()
{
    db().exec("COMMIT TRANSACTION");
}
}
