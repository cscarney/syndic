#include "feeddatabase.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <Syndication/Item>
#include <Syndication/Person>

namespace FeedCore {

static const QString feed_fields =
        QStringLiteral("Feed.id, Feed.source, Feed.localId, Feed.displayName, Feed.url, COUNT(Item.id)");
static const QString feed_join =
        QStringLiteral("Feed LEFT JOIN Item ON Item.feed=Feed.id AND Item.isRead=false");

class SQLiteFeed : public Feed {
    Q_OBJECT
public:
    qint64 m_id;

    void updateFromQuery(const QSqlQuery &query)
    {
        populateName(query.value(3).toString());
        populateUrl(query.value(4).toString());
        populateUnreadCount(query.value(5).toInt());
    }

    void populateNew(const QUrl &url, const QString &name)
    {
        populateName(name);
        populateUrl(url);
        populateUnreadCount(0);
    }

    static QSharedPointer<SQLiteFeed> forId(qint64 feedId)
    {
        static QHash<qint64, QWeakPointer<SQLiteFeed>> instances;
        auto &instance = instances[feedId];
        if (instance.isNull()) {
            auto newFeed = QSharedPointer<SQLiteFeed>(new SQLiteFeed(feedId));
            instance = newFeed;
            return newFeed;
        }
        return instance.toStrongRef();
    };

    static QSharedPointer<SQLiteFeed> fromQuery(const QSqlQuery &q)
    {
        qint64 id = q.value(0).toLongLong();
        const auto &result = forId(id);
        result->updateFromQuery(q);
        return result;
    }

private:

    SQLiteFeed(qint64 feedId) {
        m_id = feedId;
    }
};

static inline QString filePath(QString const &fileName)
{
    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!appDataDir.mkpath(".")) {
        qDebug("failed to create data dir");
        appDataDir = QDir(".");
    }
    return appDataDir.filePath(fileName);
}


static const QString db_name = QStringLiteral("FeedDatabase");

static inline QSqlDatabase db()
{
    return QSqlDatabase::database(db_name);
}

static inline int getVersion(QSqlDatabase &db)
{
    QSqlQuery q("PRAGMA user_version", db);
    if (!q.next()) {
        qDebug("getVersion returned empty set");
        return 0;
    }

    return q.value(0).toInt();
}

static inline bool exec(QSqlDatabase &db, const QString& queryString)
{
    QSqlQuery q(db);
    if (!q.exec(queryString)) {
        qDebug() << "SQL Error: " + q.lastError().text();
        return false;
    }
    return true;
}

static inline bool exec(QSqlDatabase &db, const QVector<QString> &queries)
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
        QStringLiteral("id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred");

static inline void cursorToStruct(QSqlQuery &q, StoredItem &result)
{
    qint64 feedId = q.value(1).toLongLong();
    const auto &feed = SQLiteFeed::forId(feedId);
    result = {
        .id = q.value(0).toLongLong(),
        .feedId = feed,
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

static inline void cursorToStruct(QSqlQuery &q, FeedRef &result)
{
    result = SQLiteFeed::fromQuery(q);
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

static const QString select_sort = QStringLiteral("ORDER BY date DESC");

QVector<StoredItem> FeedDatabase::selectAllItems()
{
    QSqlQuery q(db());
    q.prepare("SELECT "+item_fields+" FROM Item "+select_sort);
    return performQuery<StoredItem>(q);
}

QVector<StoredItem> FeedDatabase::selectUnreadItems()
{
    QSqlQuery q(db());
    q.prepare(
        "SELECT "+item_fields+" FROM Item "
        "WHERE isRead=0 " + select_sort);
    return performQuery<StoredItem>(q);
}

QVector<StoredItem> FeedDatabase::selectItemsByFeed(const FeedRef &feed)
{
    auto dfeed = feed.staticCast<SQLiteFeed>();
    assert(!dfeed.isNull());
    QSqlQuery q(db());
    q.prepare(
        "SELECT "+item_fields+" FROM Item "
        "WHERE feed=:feed "+select_sort);
    q.bindValue(":feed", dfeed->m_id);
    return performQuery<StoredItem>(q);
}

QVector<StoredItem> FeedDatabase::selectUnreadItemsByFeed(const FeedRef &feed)
{
    auto dfeed = feed.staticCast<SQLiteFeed>();
    assert(!dfeed.isNull());
    QSqlQuery q(db());
    q.prepare(
        "SELECT "+item_fields+" FROM Item "
        "WHERE feed=:feed AND isRead=0 "+select_sort);
    q.bindValue(":feed", dfeed->m_id);
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
    }

    StoredItem i;
    cursorToStruct(q,i);
    return i;
}

StoredItem FeedDatabase::selectItem(qint64 feed, const QString &localId)
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
    }

    StoredItem i;
    cursorToStruct(q,i);
    return i;
}

std::optional<qint64> FeedDatabase::selectItemId(const FeedRef &feed, const QString &localId)
{
    const auto &dfeed = feed.objectCast<SQLiteFeed>();
    assert(!dfeed.isNull());
    QSqlQuery q(db());
    q.prepare(
                "SELECT id FROM Item "
                "WHERE feed=:feed AND localId=:localId;");
    q.bindValue(":feed", dfeed->m_id);
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



void FeedDatabase::insertItem(StoredItem &item)
{
    const auto &id = item.feedId.objectCast<SQLiteFeed>();
    assert(!id.isNull());
    QSqlQuery q(db());
    q.prepare(
                "INSERT INTO Item (id, feed, localId, headline, author, date, url, feedContent, isRead, isStarred) "
                "VALUES (:id, :feed, :localId, :headline, :author, :date, :url, :feedContent, :isRead, :isStarred);");
    q.bindValue(":feed", id->m_id);
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

QVector<FeedRef> FeedDatabase::selectAllFeeds()
{
    QSqlQuery q(db());
    q.prepare(
                "SELECT "+feed_fields+" FROM "+feed_join+
                " GROUP BY Feed.id");
    return performQuery<FeedRef>(q);
}

void FeedDatabase::selectFeed(const FeedRef& feed)
{
    auto dfeed = feed.objectCast<SQLiteFeed>();
    QSqlQuery q(db());
    q.prepare("SELECT "+feed_fields+" FROM "+feed_join+
              " WHERE Feed.id=:id");
    q.bindValue(":id", dfeed->m_id);
    if (!q.exec()) {
        qDebug() << "SQL Error in selectFeed: " + q.lastError().text();
        return;
    }
    if (!q.next()) {
        qDebug("selectFeed for non-existent id");
        return;
    }
    dfeed->updateFromQuery(q);
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

FeedRef FeedDatabase::insertFeed(const QUrl& url)
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
        return FeedRef();
    }
    const qint64 id = q.lastInsertId().toLongLong();
    const auto &result = SQLiteFeed::forId(id);
    result->populateNew(url, urlHost);
    return result;
}

void FeedDatabase::updateFeed(const FeedRef &feed)
{
    auto dfeed = feed.objectCast<SQLiteFeed>();
    if (dfeed.isNull()) {
        qDebug() << "updating a feed that is not in the database!";
        return;
    }

    QSqlQuery q(db());
    q.prepare("UPDATE Feed SET "
              "displayName=:displayName "
              "WHERE id=:id");
    q.bindValue(":displayName", dfeed->name());
    q.bindValue(":id", dfeed->m_id);
    if (!q.exec()){
        qDebug() << "SQL Error in updateFeed: " << q.lastError().text();
    }
}

StoredItem FeedDatabase::makeStoredItem(const Syndication::ItemPtr &item, const FeedRef &feed)
{
    const auto &authors = item->authors();
    const auto &authorName = authors.empty() ? "" : authors[0]->name();
    const auto &date = QDateTime::fromTime_t(item->dateUpdated());
    auto content = item->content();
    if (content.isEmpty()) {
        content = item->description();
    }
    return {
        .id = 0, // overwritten below
        .feedId = feed,
        .localId = item->id(),
        .headers = {
            .headline = item->title(),
            .author = authorName,
            .date = date,
            .url = item->link()
        },
        .content = content,
        .status = {
            .isRead = false,
            .isStarred = false
        }
    };
}

}

#include "feeddatabase.moc"
