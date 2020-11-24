#include "feeddatabase.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

static inline QString filePath(QString const &fileName)
{
    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!appDataDir.mkpath(".")) {
        qDebug("failed to create data dir");
        appDataDir = QDir(".");
    }
    return appDataDir.filePath(fileName);
}


static const QString db_name = "FeedDatabase";

static inline QSqlDatabase db()
{
    return QSqlDatabase::database(db_name);
}

static inline int getVersion(QSqlDatabase &db)
{
    QSqlQuery q("PRAGMA user_version", db);
    if (q.next()) {
        return q.value(0).toInt();
    } else {
        qDebug("getVersion returned empty set");
        return 0;
    }
}

static inline bool exec(QSqlDatabase &db, QString queryString)
{
    QSqlQuery q(db);
    if (!q.exec(queryString)) {
        qDebug() << "SQL Error: " + q.lastError().text();
        return false;
    } else {
        return true;
    }
}

static inline bool exec(QSqlDatabase &db, std::vector<QString> queries)
{
    for(auto q : queries)
    {
        if (!exec(db, q)) return false;
    }
    return true;
}

static void initDatabase(QSqlDatabase &db)
{
    auto v = getVersion(db);
    bool success = true;
    if (v==0) {
        success = exec(db,{
                         "CREATE TABLE Feed("
                         "id INTEGER PRIMARY KEY,"
                         "source INTEGER REFERENCES FeedSource,"
                         "localId TEXT NOT NULL,"
                         "displayName TEXT,"
                         "url TEXT NOT NULL,"
                         "UNIQUE(source,localId));",

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

                           "PRAGMA user_version = 1;"
                     });
    }
    if (!success) {
        qDebug("Database initialization failed!");
        db.close();
    }
}

FeedDatabase::FeedDatabase()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE", db_name);
    db.setDatabaseName(filePath("feeds.db"));
    if (!db.open()) {
        qDebug("Failed to open database!");
    } else {
        initDatabase(db);
    }
}

static const QString item_fields =
        "id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred";

static inline void cursorToStruct(QSqlQuery &q, StoredItem &result)
{
    result = {
        .id = q.value(0).toLongLong(),
        .feedId = q.value(1).toLongLong(),
        .localId  = q.value(2).toString(),
        .headers = {
            .headline = q.value(3).toString(),
            .author = q.value(4).toString(),
            .date = QDateTime::fromSecsSinceEpoch(q.value(5).toLongLong()),
            .url = QUrl(q.value(6).toString())
        },
        .content = q.value(7).toString(),
        .status = {
            .isRead = q.value(8).toBool(),
            .isStarred = q.value(9).toBool()
        }
    };
}

static const QString feed_fields =
        "id, source, localId, displayName, url";

static inline void cursorToStruct(QSqlQuery &q, StoredFeed &result)
{
    result = {
        .id = q.value(0).toLongLong(),
        .sourceId = q.value(1).toLongLong(),
        .localId = q.value(2).toString(),
        .headers = {
            .name = q.value(3).toString(),
            .url = q.value(4).toString()
        }
    };
}

template<typename T>
static inline QVector<T> performQuery(QSqlQuery q)
{
    if (!q.exec()) {
        qDebug() << "SQL Error: " + q.lastError().text();
        return QVector<T>();
    }
    QVector<T> v;
    while (q.next())
    {
        T i;
        cursorToStruct(q, i);
        v.append(i);
    }
    return v;
}

QVector<StoredItem> FeedDatabase::selectAllItems()
{
    QSqlQuery q(db());
    q.prepare("SELECT "+item_fields+" FROM Item;");
    return performQuery<StoredItem>(q);
}

QVector<StoredItem> FeedDatabase::selectUnreadItems()
{
    QSqlQuery q(db());
    q.prepare(
        "SELECT "+item_fields+" FROM Item "
        "WHERE isRead=0");
    return performQuery<StoredItem>(q);
}

QVector<StoredItem> FeedDatabase::selectItemsByFeed(qint64 feedId)
{
    QSqlQuery q(db());
    q.prepare(
        "SELECT "+item_fields+" FROM Item "
        "WHERE feed=:feed");
    q.bindValue(":feed", feedId);
    return performQuery<StoredItem>(q);
}

QVector<StoredItem> FeedDatabase::selectUnreadItemsByFeed(qint64 feedId)
{
    QSqlQuery q(db());
    q.prepare(
        "SELECT "+item_fields+" FROM Item "
        "WHERE feed=:feed AND isRead=0");
    q.bindValue(":feed", feedId);
    return performQuery<StoredItem>(q);
}

StoredItem FeedDatabase::selectItem(qint64 id)
{
    QSqlQuery q(db());
    q.prepare(
                "SELECT "+item_fields+" FROM Item "
                "WHERE id=:id;");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectItem: " + q.lastError().text();
        return {};
    }
    if (!q.next()) {
        qDebug("selectItem for non-existent id");
        return {};
    } else {
        StoredItem i;
        cursorToStruct(q,i);
        return i;
    }
}

StoredItem FeedDatabase::selectItem(qint64 feed, QString localId)
{
    QSqlQuery q(db());
    q.prepare(
                "SELECT "+item_fields+" FROM Item "
                "WHERE feed=:feed AND localId=:localId;");
    q.bindValue(":feed", feed);
    q.bindValue(":localId", localId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectItem: " + q.lastError().text();
        return {};
    }
    if (!q.next()) {
        qDebug("selectItem for non-existent localId");
        return {};
    } else {
        StoredItem i;
        cursorToStruct(q,i);
        return i;
    }
}

std::optional<qint64> FeedDatabase::selectItemId(qint64 feed, QString localId)
{
    QSqlQuery q(db());
    q.prepare(
                "SELECT id FROM Item "
                "WHERE feed=:feed AND localId=:localId;");
    q.bindValue(":feed", feed);
    q.bindValue(":localId", localId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectItemId: " + q.lastError().text();
        return std::nullopt;
    }
    if (!q.next()) {
        return std::nullopt;
    } else {
        return q.value(0).toLongLong();
    }
}



void FeedDatabase::insertItem(StoredItem &item)
{
    QSqlQuery q(db());
    q.prepare(
                "INSERT INTO Item (id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred) "
                "VALUES (:id, :feed, :localId, :headline, :author, :date, :url, :feedContent, :isRead, :isStarred);");
    q.bindValue(":feed", item.feedId);
    q.bindValue(":localId", item.localId);
    q.bindValue(":headline", item.headers.headline);
    q.bindValue(":author", item.headers.author);
    q.bindValue(":date", item.headers.date.toSecsSinceEpoch());
    q.bindValue(":url", item.headers.url.toString());
    q.bindValue(":feedContent", item.content);
    q.bindValue(":isRead", item.status.isRead);
    q.bindValue(":isStarred", item.status.isStarred);
    if (!q.exec()) {
        qDebug() << "SQL Error in insertItem: " + q.lastError().text();
    } else {
        item.id = q.lastInsertId().toLongLong();
    }
}

void FeedDatabase::updateItemHeaders(qint64 id, FeedItemHeaders const &headers)
{
    QSqlQuery q(db());
    q.prepare(
                "UPDATE Item SET "
                "headline=:headline,"
                "author=:author,"
                "date=:date,"
                "url=:url "
                "WHERE id=:id;");
    q.bindValue(":headline", headers.headline);
    q.bindValue(":author", headers.author);
    q.bindValue(":date", headers.date.toSecsSinceEpoch());
    q.bindValue(":url", headers.url.toString());
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "SQL Error in updateItemHeaders: " + q.lastError().text();
    }
}

void FeedDatabase::updateItemContent(qint64 id, QString content)
{
    QSqlQuery q(db());
    q.prepare(
                "UPDATE Item SET "
                "feedContent=:feedContent "
                "WHERE id=:id;");
    q.bindValue(":feedContent", content);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "SQL Error in updateItemContent: " + q.lastError().text();
    }
}

void FeedDatabase::updateItemRead(qint64 id, bool isRead)
{
    qDebug() << "updateItemRead called for id " + QString::number(id);
    QSqlQuery q(db());
    q.prepare(
                "UPDATE Item SET "
                "isRead=:isRead "
                "WHERE id=:id;");
    q.bindValue(":isRead", isRead);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "SQL Error in updateItemRead: " + q.lastError().text();
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
        qDebug() << "SQL Error in updateItemStarred: " + q.lastError().text();
    }
}

QVector<StoredFeed> FeedDatabase::selectAllFeeds()
{
    QSqlQuery q(db());
    q.prepare("SELECT "+feed_fields+" FROM Feed");
    return performQuery<StoredFeed>(q);
}

StoredFeed FeedDatabase::selectFeed(qint64 id)
{
    QSqlQuery q(db());
    q.prepare(
                "SELECT "+feed_fields+" FROM Feed "
                "WHERE id=:id;");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectFeed: " + q.lastError().text();
        return {};
    }
    if (!q.next()) {
        qDebug("selectFeed for non-existent id");
        return {};
    } else {
        StoredFeed i;
        cursorToStruct(q,i);
        return i;
    }
}

StoredFeed FeedDatabase::selectFeed(qint64 source, QString localId)
{
    QSqlQuery q(db());
    q.prepare(
                "SELECT "+feed_fields+" FROM Feed "
                "WHERE source=:source AND localId=:localId;");
    q.bindValue(":source", source);
    q.bindValue(":localId", localId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectFeed: " + q.lastError().text();
        return {};
    }
    if (!q.next()) {
        qDebug("selectFeed for non-existent localId");
        return {};
    } else {
        StoredFeed i;
        cursorToStruct(q,i);
        return i;
    }
}

std::optional<qint64> FeedDatabase::selectFeedId(qint64 source, QString localId)
{
    QSqlQuery q(db());
    q.prepare(
                "SELECT id FROM Feed "
                "WHERE source=:source AND localId=:localId;");
    q.bindValue(":source", source);
    q.bindValue(":localId", localId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectFeedId: " + q.lastError().text();
        return std::nullopt;
    }
    if (!q.next()) {
        return std::nullopt;
    } else {
        return q.value(0).toLongLong();
    }
}

void FeedDatabase::insertFeed(StoredFeed &feed)
{
    QSqlQuery q(db());
    q.prepare("INSERT INTO Feed (source, localId, displayName, url) "
              "VALUES (:source, :localId, :displayName, :url);");
    q.bindValue(":source", feed.sourceId);
    q.bindValue(":localId", feed.localId);
    q.bindValue(":displayName", feed.headers.name);
    q.bindValue(":url", feed.headers.url);
    if (!q.exec()) {
        qDebug() << "SQL Error in insertFeed: " + q.lastError().text();
    } else {
        feed.id = q.lastInsertId().toLongLong();
    }
}
