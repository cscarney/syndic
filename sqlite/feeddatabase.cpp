/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "feeddatabase.h"
#include "urlmatch.h"
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

// localId of an item that was saved by the user rather than received from its feed
static const QString kSavedItemLocalIdPrefix = QStringLiteral("syndic-saved:");

static bool exec(QSqlDatabase &db, const QString &queryString)
{
    QSqlQuery q(db);
    if (!q.exec(queryString)) {
        qWarning() << "SQL Error: " + q.lastError().text();
        return false;
    }
    return true;
}

static bool exec(QSqlDatabase &db, const QList<QString> &queries)
{
    for (const auto &q : queries) {
        if (!exec(db, q)) {
            return false;
        }
    }
    return true;
}

static bool backfillUrlKeys(QSqlDatabase &db)
{
    QList<QPair<qint64, QString>> keys;
    {
        QSqlQuery q(db);
        if (!q.exec("SELECT id, url FROM Item")) {
            qWarning() << "SQL Error in backfillUrlKeys: " + q.lastError().text();
            return false;
        }
        while (q.next()) {
            keys.append({q.value(0).toLongLong(), FeedCore::UrlMatch::normalizedUrlKey(q.value(1).toUrl())});
        }
    }

    db.transaction();
    QSqlQuery update(db);
    update.prepare("UPDATE Item SET urlKey=:urlKey WHERE id=:id");
    for (const auto &[id, key] : std::as_const(keys)) {
        update.bindValue(":urlKey", key);
        update.bindValue(":id", id);
        if (!update.exec()) {
            qWarning() << "SQL Error in backfillUrlKeys: " + update.lastError().text();
            db.rollback();
            return false;
        }
    }
    return db.commit();
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
        // fall through

    case 2:
        success = success
            && exec(db,
                    {"ALTER TABLE Item "
                     "ADD COLUMN urlKey TEXT;",

                     "ALTER TABLE Item "
                     "ADD COLUMN preferReadable INTEGER DEFAULT 0;",

                     "CREATE INDEX Item_urlKey ON Item(urlKey);"})
            && backfillUrlKeys(db) && exec(db, "PRAGMA user_version = 3;");
        break;

    case 3:
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

ItemQuery FeedDatabase::selectUnfiledItems()
{
    ItemQuery q(db(), "feed IS NULL " + select_sort);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectUnfiledItems: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItemsBySearch(const QString &search)
{
    QString like = "%" + search + "%";
    ItemQuery q(db(), "headline LIKE :search OR feedContent LIKE :search OR readableContent LIKE :search " + select_sort);
    q.bindValue(":search", like);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItemsBySearch: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItemsByRecommended(int limit)
{
    ItemQuery q(db());
    q.prepare(
                "SELECT "+
                    ItemQuery::fieldList() + ", "
                    "ROW_NUMBER() over (PARTITION BY feed ORDER BY date DESC) AS feedRank "
                "FROM Item "
                "ORDER BY isRead ASC, feedRank ASC, date ASC "
                "LIMIT :limit;"
                );
    q.bindValue(":limit", limit);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItemsByRecommended: " + q.lastError().text();
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
        "INSERT INTO Item (id, feed, localId, headline, author, date, url, urlKey, feedContent, isRead, isStarred) "
        "VALUES (:id, :feed, :localId, :headline, :author, :date, :url, :urlKey, :feedContent, :isRead, :isStarred);");
    q.bindValue(":feed", feedId);
    q.bindValue(":localId", localId);
    q.bindValue(":headline", title);
    q.bindValue(":author", author);
    q.bindValue(":date", qint64(date));
    q.bindValue(":url", url.toString());
    q.bindValue(":urlKey", FeedCore::UrlMatch::normalizedUrlKey(url));
    q.bindValue(":feedContent", content);
    q.bindValue(":isRead", false);
    q.bindValue(":isStarred", false);
    if (!q.exec()) {
        qWarning() << "SQL Error in insertItem: " + q.lastError().text();
        return std::nullopt;
    }
    return q.lastInsertId().toLongLong();
}

std::optional<qint64>
FeedDatabase::insertSavedItem(std::optional<qint64> feedId, const QString &title, const QString &author, time_t date, const QUrl &url, const QString &content)
{
    QSqlQuery q(db());
    q.prepare(
        "INSERT INTO Item (feed, localId, headline, author, date, url, urlKey, feedContent, readableContent, isRead, isStarred, preferReadable) "
        "VALUES (:feed, :localId, :headline, :author, :date, :url, :urlKey, :content, :content, 1, 1, 1);");
    q.bindValue(":feed", feedId ? QVariant(*feedId) : QVariant(QMetaType::fromType<qint64>()));
    q.bindValue(":localId", kSavedItemLocalIdPrefix + url.toString());
    q.bindValue(":headline", title);
    q.bindValue(":author", author);
    q.bindValue(":date", qint64(date));
    q.bindValue(":url", url.toString());
    q.bindValue(":urlKey", FeedCore::UrlMatch::normalizedUrlKey(url));
    q.bindValue(":content", content);
    if (!q.exec()) {
        qWarning() << "SQL Error in insertSavedItem: " + q.lastError().text();
        return std::nullopt;
    }
    return q.lastInsertId().toLongLong();
}

ItemQuery FeedDatabase::selectItemsByUrlKey(const QString &urlKey)
{
    ItemQuery q(db(), "urlKey=:urlKey " + select_sort);
    q.bindValue(":urlKey", urlKey);
    if (!q.exec()) {
        qWarning() << "SQL Error in selectItemsByUrlKey: " + q.lastError().text();
    }
    return q;
}

std::optional<qint64> FeedDatabase::adoptSavedItem(qint64 feedId, const QString &urlKey, const QString &localId)
{
    QSqlQuery q(db());
    q.prepare(
        "SELECT id FROM Item "
        "WHERE feed=:feed AND urlKey=:urlKey AND localId LIKE :savedPattern "
        "ORDER BY date DESC LIMIT 1;");
    q.bindValue(":feed", feedId);
    q.bindValue(":urlKey", urlKey);
    q.bindValue(":savedPattern", kSavedItemLocalIdPrefix + QLatin1Char('%'));
    if (!q.exec()) {
        qWarning() << "SQL Error in adoptSavedItem: " + q.lastError().text();
        return std::nullopt;
    }
    if (!q.next()) {
        return std::nullopt;
    }
    const qint64 id = q.value(0).toLongLong();

    QSqlQuery update(db());
    update.prepare("UPDATE Item SET localId=:localId WHERE id=:id;");
    update.bindValue(":localId", localId);
    update.bindValue(":id", id);
    if (!update.exec()) {
        qWarning() << "SQL Error in adoptSavedItem: " + update.lastError().text();
        return std::nullopt;
    }
    return id;
}

void FeedDatabase::updateItemHeaders(qint64 id, const QString &title, const QString &author, const QUrl &url)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "headline=:headline,"
        "author=:author,"
        "url=:url,"
        "urlKey=:urlKey "
        "WHERE id=:id;");
    q.bindValue(":headline", title);
    q.bindValue(":author", author);
    q.bindValue(":url", url.toString());
    q.bindValue(":urlKey", FeedCore::UrlMatch::normalizedUrlKey(url));
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

void FeedDatabase::updateItemSavedContent(qint64 id, const QString &readableContent)
{
    QSqlQuery q(db());
    q.prepare(
        "UPDATE Item SET "
        "readableContent=:readableContent,"
        "preferReadable=1 "
        "WHERE id=:id;");
    q.bindValue(":readableContent", readableContent);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "SQL Error in updateItemSavedContent: " + q.lastError().text();
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
        q.bindValue(":lastUpdate", QVariant(QMetaType::fromType<qint64>()));
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
