#include "feeddatabase.h"
#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


namespace Sqlite {

static const QString db_name_fmt = QStringLiteral("Sqlite_FeedDatabase_%1");

QSqlDatabase FeedDatabase::db()
{
    return QSqlDatabase::database(m_dbName);
}

static int getVersion(QSqlDatabase &db)
{
    QSqlQuery q("PRAGMA user_version", db);
    if (!q.next()) {
        qDebug("getVersion returned empty set");
        return 0;
    }

    return q.value(0).toInt();
}

static bool exec(QSqlDatabase &db, const QString& queryString)
{
    QSqlQuery q(db);
    if (!q.exec(queryString)) {
        qDebug() << "SQL Error: " + q.lastError().text();
        return false;
    }
    return true;
}

static bool exec(QSqlDatabase &db, const QVector<QString> &queries)
{
    for(const auto &q : queries) {
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
    if (v==0) {
        success = exec(db,{
                         "CREATE TABLE Feed("
                         "id INTEGER PRIMARY KEY,"
                         "source INTEGER REFERENCES FeedSource,"
                         "localId TEXT NOT NULL,"
                         "displayName TEXT,"
                         "url TEXT NOT NULL,"
                         "link TEXT NOT NULL,"
                         "updateInterval INTEGER,"
                         "lastUpdate INTEGER,"
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

FeedDatabase::FeedDatabase(QString filePath)
{
    static int dbCount {0};
    m_dbName = db_name_fmt.arg(++dbCount);
    auto db = QSqlDatabase::addDatabase("QSQLITE", m_dbName);
    db.setDatabaseName(filePath);
    if (!db.open()) {
        qDebug("Failed to open database!");
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
    ItemQuery q(db(), "1 "+select_sort);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectAllItems: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectUnreadItems()
{
    ItemQuery q(db(), "isRead=0 "+select_sort);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectUnreadItems: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItemsByFeed(qint64 feedId)
{
    ItemQuery q(db(), "feed=:feed "+select_sort);
    q.bindValue(":feed", feedId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectItemsByFeed: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectUnreadItemsByFeed(qint64 feedId)
{
    ItemQuery q(db(), "feed=:feed AND isRead=0 "+select_sort);
    q.bindValue(":feed", feedId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectUnreadItemsByFeed: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItem(qint64 id)
{
    ItemQuery q(db(), "id=:id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectItem: " + q.lastError().text();
    }
    return q;
}

ItemQuery FeedDatabase::selectItem(qint64 feed, const QString &localId)
{
    ItemQuery q(db(), "feed=:feed AND localId=:localId");
    q.bindValue(":feed", feed);
    q.bindValue(":localId", localId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectItem: " + q.lastError().text();
    }
    return q;
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
        qDebug() << "SQL Error in selectItemId: " + q.lastError().text();
        return std::nullopt;
    }
    if (!q.next()) {
        return std::nullopt;
    }
    return q.value(0).toLongLong();
}



std::optional<qint64> FeedDatabase::insertItem(qint64 feedId, const QString &localId, const QString &title, const QString &author, const QDateTime &date, const QUrl &url, const QString &content)
{
    QSqlQuery q(db());
    q.prepare(
                "INSERT INTO Item (id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred) "
                "VALUES (:id, :feed, :localId, :headline, :author, :date, :url, :feedContent, :isRead, :isStarred);");
    q.bindValue(":feed", feedId);
    q.bindValue(":localId", localId);
    q.bindValue(":headline", title);
    q.bindValue(":author", author);
    q.bindValue(":date", date.toSecsSinceEpoch());
    q.bindValue(":url", url.toString());
    q.bindValue(":feedContent", content);
    q.bindValue(":isRead", false);
    q.bindValue(":isStarred", false);
    if (!q.exec()) {
        qDebug() << "SQL Error in insertItem: " + q.lastError().text();
        return std::nullopt;
    }
    return q.lastInsertId().toLongLong();
}

void FeedDatabase::updateItemHeaders(qint64 id, const QString &title, const QDateTime &date, const QString &author, const QUrl &url)
{
    QSqlQuery q(db());
    q.prepare(
                "UPDATE Item SET "
                "headline=:headline,"
                "author=:author,"
                "date=:date,"
                "url=:url "
                "WHERE id=:id;");
    q.bindValue(":headline", title);
    q.bindValue(":author", author);
    q.bindValue(":date", date.toSecsSinceEpoch());
    q.bindValue(":url", url.toString());
    q.bindValue(":id", id);
    if (!q.exec()) {
        qDebug() << "SQL Error in updateItemHeaders: " + q.lastError().text();
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
        qDebug() << "SQL Error in updateItemContent: " + q.lastError().text();
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

void FeedDatabase::deleteItemsForFeed(qint64 feedId)
{
    QSqlQuery q(db());
    q.prepare(
                "DELETE FROM Item WHERE feed=:feed");
    q.bindValue(":feed", feedId);
    if (!q.exec()) {
        qDebug() << "SQL Error in deleteItemsForFeed: " << q.lastError().text();
    }
}

FeedQuery FeedDatabase::selectAllFeeds()
{
    FeedQuery q(db(), "1");
    if (!q.exec()) {
        qDebug() << "SQL Error: " + q.lastError().text();
    }
    return q;
}

FeedQuery FeedDatabase::selectFeed(qint64 feedId)
{
    FeedQuery q(db(), "Feed.id=:id");
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectFeed: " + q.lastError().text();
    }
    return q;
}

std::optional<qint64> FeedDatabase::selectFeedId(qint64 source, const QString &localId)
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
    }

    return q.value(0).toLongLong();
}

std::optional<qint64> FeedDatabase::insertFeed(const QUrl& url)
{

    QSqlQuery q(db());
    const QString &urlString = url.toString();
    const QString &urlHost = url.host();
    q.prepare("INSERT INTO Feed (source, localId, displayName, url) "
              "VALUES (:source, :localId, :displayName, :url);");
    q.bindValue(":source", 0);
    q.bindValue(":localId", urlString);
    q.bindValue(":displayName", urlHost);
    q.bindValue(":url", urlString);
    if (!q.exec()) {
        qDebug() << "SQL Error in insertFeed: " + q.lastError().text();
        return std::nullopt;
    }
    return q.lastInsertId().toLongLong();
}

void FeedDatabase::updateFeedName(qint64 feedId, const QString &newName)
{
    QSqlQuery q(db());
    q.prepare("UPDATE Feed SET "
              "displayName=:displayName "
              "WHERE id=:id");
    q.bindValue(":displayName", newName);
    q.bindValue(":id", feedId);
    if (!q.exec()){
        qDebug() << "SQL Error in updateFeed: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedLink(qint64 feedId, const QString &link)
{
    QSqlQuery q(db());
    q.prepare("UPDATE Feed SET "
              "link=:link "
              "WHERE id=:id");
    q.bindValue(":link", link);
    q.bindValue(":id", feedId);
    if (!q.exec()){
        qDebug() << "SQL Error in updateFeed: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedUpdateInterval(qint64 feedId, qint64 updateInterval)
{
    QSqlQuery q(db());
    q.prepare("UPDATE Feed SET "
              "updateInterval=:updateInterval "
              "WHERE id=:id");
    q.bindValue(":updateInterval", updateInterval);
    q.bindValue(":id", feedId);
    if (!q.exec()){
        qDebug() << "SQL Error in updateFeed: " << q.lastError().text();
    }
}

void FeedDatabase::updateFeedLastUpdate(qint64 feedId, QDateTime lastUpdate)
{
    QSqlQuery q(db());
    q.prepare("UPDATE Feed SET "
              "lastUpdate=:lastUpdate "
              "WHERE id=:id");
    if (lastUpdate.isValid()) {
        q.bindValue(":lastUpdate", lastUpdate.toSecsSinceEpoch());
    } else {
        q.bindValue(":lastUpdate", QVariant(QVariant::LongLong));
    }
    q.bindValue(":id", feedId);
    if (!q.exec()){
        qDebug() << "SQL Error in updateFeed: " << q.lastError().text();
    }
}

void FeedDatabase::deleteFeed(qint64 feedId)
{
    QSqlQuery q(db());
    q.prepare(
                "DELETE FROM Feed WHERE id=:id");
    q.bindValue(":id", feedId);
    if (!q.exec()) {
        qDebug() << "SQL Error in deleteFeed: " << q.lastError().text();
    }
}

}
