#include "basicfeedstorage.h"
#include <qstandardpaths.h>
#include <qdir.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

static const QString itemsFileName = "items.json";
static const QString feedsFileName = "feeds.json";

static inline QString filePath(QString const &fileName)
{
    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!appDataDir.mkpath(".")) {
        qDebug("failed to create data dir");
        appDataDir = QDir(".");
    }
    return appDataDir.filePath(fileName);
}

static inline FeedSource::Item deserializeItem(QJsonObject const &obj)
{
    return FeedSource::Item {
        .id = obj["id"].toInt(),
        .feedId = obj["feed_id"].toInt(),
        .headline = obj["headline"].toString(),
        .author = obj["author"].toString(),
        .date = QDateTime::fromSecsSinceEpoch(obj["date"].toInt()),
        .content = obj["content"].toString(),
        .url = obj["url"].toString(),
        .isUnread = obj["isUnread"].toBool(),
        .isStarred = obj["isStarred"].toBool()
    };
}

static inline QJsonObject serializeItem(FeedSource::Item const &item)
{
    return QJsonObject {
        {"id", item.id},
        {"feed_id", item.feedId},
        {"headline",item.headline},
        {"author",item.author},
        {"date", item.date.toSecsSinceEpoch()},
        {"content",item.content},
        {"url",item.url},
        {"isUnread", item.isUnread},
        {"isStarred", item.isStarred}
    };
}

static inline QJsonObject serializeFeed(FeedSource::Feed const &feed)
{
    return QJsonObject {
        {"id", feed.id},
        {"name", feed.name}
    };
}

static inline FeedSource::Feed deserializeFeed(QJsonObject const &obj)
{
    return FeedSource::Feed {
        .id=obj["id"].toInt(),
        .name=obj["name"].toString()
    };
}

static void loadFeeds(QList<FeedSource::Feed> &feeds)
{
    QFile feedsFile(filePath(feedsFileName));
    if (feedsFile.exists()) {
        feedsFile.open(QFile::ReadOnly);
        auto feedsDoc = QJsonDocument::fromJson(feedsFile.readAll());
        auto feedsList = feedsDoc.array();
        for (auto i=feedsList.constBegin();i!=feedsList.constEnd(); ++i){
            FeedSource::Feed feed = deserializeFeed(i->toObject());
            feeds.append(feed);
        }
    }
}

static void loadItems(QHash<qint64, FeedSource::Item> &itemsById)
{
    QFile itemsFile(filePath(itemsFileName));
    if (itemsFile.exists()) {
        itemsFile.open(QFile::ReadOnly);
        auto itemsDoc = QJsonDocument::fromJson(itemsFile.readAll());
        auto itemsList = itemsDoc.array();
        for (auto i=itemsList.constBegin();i!=itemsList.constEnd(); ++i){
            FeedSource::Item item = deserializeItem(i->toObject());
            itemsById[item.id] = item;
        }
    }
}

BasicFeedStorage::BasicFeedStorage(QObject *parent)
    : FeedStorage(parent)
{
    loadFeeds(m_feeds);
    loadItems(m_itemsById);
}

BasicFeedStorage::~BasicFeedStorage(){
    try {
        save();
    } catch (...) {
        qDebug("failed to save");
    }
}

FeedSource::Item BasicFeedStorage::getById(qint64 id)
{
    return m_itemsById.value(id, {});
}

qint64 BasicFeedStorage::storeItem(FeedSource::Item const &item)
{
    if (item.content.isNull()) {
        auto itemWithContent = item;
        itemWithContent.content = m_itemsById[item.id].content;
        m_itemsById[item.id] = itemWithContent;
    } else {
        m_itemsById[item.id] = item;
    }
    return item.id;
}

QList<FeedSource::Item> BasicFeedStorage::getAll()
{
    return m_itemsById.values();
}

bool BasicFeedStorage::storeFeed(FeedSource::Feed feed)
{
    auto i = findFeed(feed.id);
    if (i<0) {
        m_feeds.append(feed);
        return true;
    }
    return false;
}

QList<FeedSource::Feed> BasicFeedStorage::getFeeds()
{
    return m_feeds;
}

static inline void saveItems(QHash<qint64,FeedSource::Item> const &itemsById)
{
    QJsonArray items;
    for(auto i=itemsById.constBegin();i!=itemsById.constEnd();++i) {
        items.append(serializeItem(*i));
    }

    QFile jsonFile(filePath(itemsFileName));
    jsonFile.open(QFile::WriteOnly);
    if (jsonFile.write(QJsonDocument(items).toJson()) < 0) {
        qDebug("failed to save...");
    } else {
        qDebug("saved to %s",qUtf8Printable(jsonFile.fileName()));
    }
}

static inline void saveFeeds(QList<FeedSource::Feed> const &feeds)
{
    QJsonArray output;
    for(auto i=feeds.constBegin();i!=feeds.constEnd();++i) {
        output.append(serializeFeed(*i));
    }

    QFile jsonFile(filePath(feedsFileName));
    jsonFile.open(QFile::WriteOnly);
    if (jsonFile.write(QJsonDocument(output).toJson()) < 0) {
        qDebug("failed to save...");
    } else {
        qDebug("saved to %s",qUtf8Printable(jsonFile.fileName()));
    }
}

void BasicFeedStorage::save()
{
    saveItems(m_itemsById);
    saveFeeds(m_feeds);
}

int BasicFeedStorage::findFeed(qint64 id)
{
    for(auto i=0; i<m_feeds.length();i++) {
        if (m_feeds[i].id == id) {
            return i;
        }
    }
    return -1;
}
