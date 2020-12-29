#include "sqlite/storageimpl.h"
#include <QVector>
#include <QTimer>
#include <QDebug>
#include <Syndication/Person>
#include "sqlite/feedimpl.h"
#include "sqlite/articleimpl.h"
#include "articleref.h"
using namespace FeedCore;
using namespace Sqlite;

void StorageImpl::appendArticleResults(Future<ArticleRef> *op, ItemQuery &q)
{
    while (q.next()) {
        const auto &feed = m_feedFactory.getInstance(q.feed(), this);
        const auto &item = m_articleFactory.getInstance(q.id(), feed);
        item->updateFromQuery(q);
        op->appendResult(item);
    }
}

Future<ArticleRef> *StorageImpl::getAll()
{
    return Future<ArticleRef>::yield(this, [this](auto *op){
        ItemQuery q { m_db.selectAllItems() };
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getUnread()
{
    return Future<ArticleRef>::yield(this, [this](auto *op){
        ItemQuery q { m_db.selectUnreadItems() };
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getById(qint64 id)
{
    return Future<ArticleRef>::yield(this, [this, id](auto *op){
        ItemQuery q { m_db.selectItem(id) };
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getByFeed(FeedImpl *feed)
{
    const qint64 feedId { feed->id() };
    return Future<ArticleRef>::yield(this, [this, feedId](auto *op){
        ItemQuery q = m_db.selectItemsByFeed(feedId);
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::getUnreadByFeed(FeedImpl *feed)
{
    const qint64 feedId = feed->id();
    return Future<ArticleRef>::yield(this, [this, feedId](auto *op){
        ItemQuery q { m_db.selectUnreadItemsByFeed(feedId) };
        appendArticleResults(op, q);
    });
}

Future<ArticleRef> *StorageImpl::storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item)
{
    const qint64 feedId { feed->id() };
    return Future<ArticleRef>::yield(this, [this, item, feedId](auto *op){
        const auto &itemId = m_db.selectItemId(feedId, item->id());
        const auto &authors = item->authors();
        const auto &authorName = authors.empty() ? "" : authors[0]->name();
        const auto &date = QDateTime::fromTime_t(item->dateUpdated());
        auto content = item->content();
        if (content.isEmpty()) {
            content = item->description();
        }

        if (itemId) {
            m_db.updateItemHeaders(*itemId, item->title(), date, authorName, item->link());
            if (!content.isEmpty()) {
                m_db.updateItemContent(*itemId, content);
            }

            // TODO this  sometimes creates an unnecessary item instance
            getById(*itemId); // push the update into any existing item instance

            op->setResult();
            return;
        }

        const auto &newId = m_db.insertItem(feedId, item->id(), item->title(), authorName, date, item->link(), content);
        if (!newId) {
            op->setResult();
            return;
        }
        ItemQuery result { m_db.selectItem(*newId) };
        appendArticleResults(op, result);
    });
}

Future<ArticleRef> *StorageImpl::updateArticleRead(ArticleImpl *article, bool isRead)
{
    const qint64 itemId { article->id() };
    const bool oldValue { article->isRead() };
    return Future<ArticleRef>::yield(this, [this, itemId, isRead, oldValue](auto *op){
        if (oldValue == isRead) {
            op->setResult() ;
        } else {
            m_db.updateItemRead(itemId, isRead);
            ItemQuery result { m_db.selectItem(itemId) };
            appendArticleResults(op, result);
        }
    });
}

void StorageImpl::appendFeedResults(Future<FeedRef> *op, FeedQuery &q)
{
    while (q.next()) {
        auto ref = m_feedFactory.getInstance(q.id(), this);
        ref->updateFromQuery(q);
        op->appendResult(ref);
    }
}

Future<FeedRef> *StorageImpl::getFeeds()
{
    return Future<FeedRef>::yield(this, [this](auto *op){
        FeedQuery q { m_db.selectAllFeeds() };
        appendFeedResults(op, q);
    });
}

Future<FeedRef> *StorageImpl::storeFeed(const QUrl &url)
{
    return Future<FeedRef>::yield(this, [this, url](auto *op){
        const auto &existingId = m_db.selectFeedId(0, url.toString());
        if (existingId) {
            qDebug() << "trying to insert feed for " << url << "which already exists";
            op->setResult();
            return;
        }
        const auto &insertId = m_db.insertFeed(url);
        if (!insertId)
        {
            op->setResult();
            return;
        }
        FeedQuery result { m_db.selectFeed(*insertId) };
        appendFeedResults(op, result);
    });
}

void StorageImpl::updateFeedMetadata(FeedImpl *storedFeed)
{
    const qint64 id = storedFeed->id();
    const QString name = storedFeed->name();
    QTimer::singleShot(0, this, [this, id, name]{
        m_db.updateFeed(id, name);
    });
}
