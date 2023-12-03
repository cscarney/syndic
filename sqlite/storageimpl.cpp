/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "sqlite/storageimpl.h"
#include "articleref.h"
#include "sqlite/articleimpl.h"
#include "sqlite/feedimpl.h"
#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QEvent>
#include <QList>
#include <QSemaphore>
#include <QTimer>
#include <Syndication/Person>
#include <utility>
using namespace FeedCore;
using namespace SqliteStorage;

// The worker class belongs to the worker thread; the *only* method
// that should ever be called from the main thread is runInDatabaseThread
class StorageImpl::Worker : public QObject
{
public:
    explicit Worker(StorageImpl *storage, const QString &filePath)
        : m_db(filePath)
        , m_storage(storage)
    {
    }

    template<typename Func>
    void runInDatabaseThread(Func func);

    template<typename Payload, typename Func>
    QFuture<Payload> runInDatabaseThread(Func func);

    template<typename Func, typename... Args>
    void runInDatabaseThread(Func func, Args... args);

    template<typename Func>
    void runOnMainThread(Func func);

    void appendArticleResults(QPromise<FeedCore::ArticleRef> &op, ItemQuery &q);
    void appendFeedResults(QPromise<FeedCore::Feed *> &op, FeedQuery &q);
    void ensureTransaction();
    bool hasArticle(qint64 id) const;

private:
    FeedDatabase m_db;
    FeedCore::ObjectFactory<qint64, FeedImpl> m_feedFactory;
    QHash<qint64, QWeakPointer<ArticleImpl>> m_articles;
    StorageImpl *m_storage;
    bool m_hasTransaction{false};
    const static int CommitEvent;
    void customEvent(QEvent *e) override;
};

class StorageImpl::WorkerThread : public QThread
{
public:
    explicit WorkerThread(QObject *parent)
        : QThread(parent)
    {
    }

private:
    void run() override
    {
        exec();
        // clear the event queue before stopping, otherwise updates may be lost
        while (eventDispatcher()->processEvents(QEventLoop::AllEvents)) { }
    }
};

const int StorageImpl::Worker::CommitEvent = QEvent::registerEventType();

void StorageImpl::Worker::appendArticleResults(QPromise<ArticleRef> &op, ItemQuery &q)
{
    while (q.next()) {
        runOnMainThread([this, &op, &q]() {
            if (auto existingArticle = m_articles[q.id()].toStrongRef()) {
                existingArticle->updateFromQuery(q);
                op.addResult(existingArticle);
            } else {
                qint64 id = q.id();
                const auto &feed = m_feedFactory.getInstance(q.feed(), m_storage);
                QSharedPointer<ArticleImpl> newArticle{new ArticleImpl(id, m_storage, feed, q)};
                m_articles[id] = newArticle;
                op.addResult(newArticle);
            }
        });
    }
}

void StorageImpl::onFeedRequestDelete(FeedImpl *feed)
{
    feed->updater()->abort();
    qint64 feedId{feed->id()};
    feed->deleteLater();
    m_worker->runInDatabaseThread([feedId](auto &m_db) {
        m_db.deleteItemsForFeed(feedId);
        m_db.deleteFeed(feedId);
    });
}

void StorageImpl::Worker::ensureTransaction()
{
    if (m_hasTransaction) {
        return;
    }
    m_db.beginTransaction();
    m_hasTransaction = true;
    QCoreApplication::postEvent(this, new QEvent(static_cast<QEvent::Type>(CommitEvent)), Qt::LowEventPriority);
}

bool StorageImpl::Worker::hasArticle(qint64 id) const
{
    return m_articles.contains(id) && !m_articles[id].isNull();
}

QFuture<ArticleRef> StorageImpl::getAll()
{
    return m_worker->runInDatabaseThread<ArticleRef>([this](auto &db, auto &op) {
        ItemQuery q{db.selectAllItems()};
        m_worker->appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getUnread()
{
    return m_worker->runInDatabaseThread<ArticleRef>([this](auto &db, auto &op) {
        ItemQuery q{db.selectUnreadItems()};
        m_worker->appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getStarred()
{
    return m_worker->runInDatabaseThread<ArticleRef>([this](auto &db, auto &op) {
        ItemQuery q{db.selectStarredItems()};
        m_worker->appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getSearchResults(const QString &search)
{
    return m_worker->runInDatabaseThread<ArticleRef>([this, search](auto &db, auto &op) {
        ItemQuery q{db.selectItemsBySearch(search)};
        m_worker->appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getHighlights()
{
    return m_worker->runInDatabaseThread<ArticleRef>([this](auto &db, auto &op) {
        ItemQuery q{db.selectItemsByRecommended()};
        m_worker->appendArticleResults(op, q);
    });
}

StorageImpl::StorageImpl(const QString &filePath)
    : m_thread{new WorkerThread(this)}
{
    m_thread->start();

    // block until the thread event loop is running
    QObject sentinel;
    sentinel.moveToThread(m_thread);
    QMetaObject::invokeMethod(
        &sentinel,
        [this, &filePath] {
            m_worker = new Worker(this, filePath);
        },
        Qt::BlockingQueuedConnection);
    QObject::connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
}

StorageImpl::~StorageImpl()
{
    m_thread->requestInterruption();
    m_thread->quit();
    m_thread->wait();
}

QFuture<ArticleRef> StorageImpl::getById(qint64 id)
{
    return m_worker->runInDatabaseThread<ArticleRef>([this, id](auto &db, auto &op) {
        ItemQuery q{db.selectItem(id)};
        m_worker->appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getByFeed(FeedImpl *feed)
{
    const qint64 feedId{feed->id()};
    return m_worker->runInDatabaseThread<ArticleRef>([this, feedId](auto &db, auto &op) {
        ItemQuery q = db.selectItemsByFeed(feedId);
        m_worker->appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::getUnreadByFeed(FeedImpl *feed)
{
    const qint64 feedId = feed->id();
    return m_worker->runInDatabaseThread<ArticleRef>([this, feedId](auto &db, auto &op) {
        ItemQuery q{db.selectUnreadItemsByFeed(feedId)};
        m_worker->appendArticleResults(op, q);
    });
}

QFuture<ArticleRef> StorageImpl::storeArticle(FeedImpl *feed, const Syndication::ItemPtr &item)
{
    // TODO maybe sync with the main thread here so we can use `item` directly instead
    // of copying everything into the lambda? We don't want to block the main thread
    // waiting for the database, but blocking the database thread waiting for a main
    // thread event would probably be OK.
    return m_worker->runInDatabaseThread<ArticleRef>([this,
                                                      feedId = feed->id(),
                                                      id = item->id(),
                                                      authorName = item->authors().empty() ? "" : item->authors()[0]->name(),
                                                      date = item->dateUpdated(),
                                                      title = item->title(),
                                                      link = item->link(),
                                                      content = item->content().isEmpty() ? item->description() : item->content()](auto &db, auto &op) {
        const auto &itemId = db.selectItemId(feedId, id);

        if (itemId) {
            // Item already exists in the database; update it and return
            // NB: we don't include  already existing items in the future results;
            //      that would cause an articleAdded signal to be emittedfrom the feed
            db.updateItemHeaders(*itemId, title, authorName, link);
            if (date > 0) {
                db.updateItemDate(*itemId, date);
            }
            if (!content.isEmpty()) {
                db.updateItemContent(*itemId, content);
            }

            // If an existing FeedCore::Article instance exists, force it to update from the db
            if (m_worker->hasArticle(*itemId)) {
                getById(*itemId);
            }
            return;
        }

        // Use the provided date if there is one, or else the time when we first see it
        time_t syntheticDate = date == 0 ? QDateTime::currentSecsSinceEpoch() : date;

        const auto &newId = db.insertItem(feedId, id, title, authorName, syntheticDate, link, content);
        if (!newId) {
            return;
        }
        ItemQuery result{db.selectItem(*newId)};
        m_worker->appendArticleResults(op, result);
    });
}

QFuture<QString> StorageImpl::getContent(ArticleImpl *article)
{
    return m_worker->runInDatabaseThread<QString>([id = article->id()](auto &db, auto &op) {
        op.addResult(db.selectItemContent(id));
    });
}

QFuture<QString> StorageImpl::getReadableContent(ArticleImpl *article)
{
    return m_worker->runInDatabaseThread<QString>([id = article->id()](auto &db, auto &op) {
        QString readableContent = db.selectItemReadableContent(id);
        if (!readableContent.isEmpty()) {
            op.addResult(readableContent);
        }
    });
}

void StorageImpl::cacheReadableContent(ArticleImpl *article, const QString &readableContent)
{
    const qint64 itemId{article->id()};
    m_worker->runInDatabaseThread([itemId, readableContent, worker = m_worker](auto &db) {
        worker->ensureTransaction();
        db.updateItemReadableContent(itemId, readableContent);
    });
}

void StorageImpl::onArticleReadChanged(ArticleImpl *article)
{
    m_worker->runInDatabaseThread([itemId = article->id(), isRead = article->isRead(), worker = m_worker](auto &db) {
        worker->ensureTransaction();
        db.updateItemRead(itemId, isRead);
    });
}

void StorageImpl::onArticleStarredChanged(ArticleImpl *article)
{
    m_worker->runInDatabaseThread([itemId = article->id(), isStarred = article->isStarred(), worker = m_worker](auto &db) {
        worker->ensureTransaction();
        db.updateItemStarred(itemId, isStarred);
    });
}

void StorageImpl::Worker::appendFeedResults(QPromise<Feed *> &op, FeedQuery &q)
{
    while (q.next()) {
        runOnMainThread([this, &op, &q]() {
            auto *ref = m_feedFactory.getInstance(q.id(), m_storage);
            ref->updateFromQuery(q);
            op.addResult(ref);
        });
    }
}

QFuture<Feed *> StorageImpl::getFeeds()
{
    return m_worker->runInDatabaseThread<Feed *>([this](auto &db, auto &op) {
        FeedQuery q{db.selectAllFeeds()};
        m_worker->appendFeedResults(op, q);
    });
}

static qint64 packModeValue(Feed::UpdateMode mode, qint64 value)
{
    switch (mode) {
    case Feed::InheritUpdateMode:
    default:
        return 0;

    case Feed::DisableUpdateMode:
        return -1;

    case Feed::OverrideUpdateMode:
        return value;
    }
}

static qint64 packFeedUpdateInterval(Feed *feed)
{
    return packModeValue(feed->updateMode(), feed->updateInterval());
}

static qint64 packFeedExpireAge(Feed *feed)
{
    return packModeValue(feed->expireMode(), feed->expireAge());
}

QFuture<Feed *> StorageImpl::storeFeed(Feed *feed)
{
    const QUrl &url = feed->url();
    const QString &name = feed->name();
    const QString &category = feed->category();
    const qint64 updateInterval = packFeedUpdateInterval(feed);
    const qint64 expireAge = packFeedExpireAge(feed);
    return m_worker->runInDatabaseThread<FeedCore::Feed *>([this, url, name, category, updateInterval, expireAge](auto &db, auto &op) {
        m_worker->ensureTransaction();
        const auto &insertId = db.insertFeed(url);
        if (!insertId) {
            return;
        }
        db.updateFeedUpdateInterval(*insertId, updateInterval);
        db.updateFeedExpireAge(*insertId, expireAge);
        db.updateFeedName(*insertId, name);
        db.updateFeedCategory(*insertId, category);
        FeedQuery result{db.selectFeed(*insertId)};
        m_worker->appendFeedResults(op, result);
    });
}

void StorageImpl::onUpdateModeChanged(FeedImpl *feed)
{
    qint64 updateInterval = packFeedUpdateInterval(feed);
    m_worker->runInDatabaseThread(&FeedDatabase::updateFeedUpdateInterval, feed->id(), updateInterval);
}

void StorageImpl::onUpdateIntervalChanged(FeedImpl *feed)
{
    if (feed->updateMode() != Feed::OverrideUpdateMode) {
        return;
    }
    m_worker->runInDatabaseThread(&FeedDatabase::updateFeedUpdateInterval, feed->id(), feed->updateInterval());
}

void StorageImpl::onExpireModeChanged(FeedImpl *feed)
{
    qint64 expireAge = packFeedExpireAge(feed);
    m_worker->runInDatabaseThread(&FeedDatabase::updateFeedExpireAge, feed->id(), expireAge);
}

void StorageImpl::onExpireAgeChanged(FeedImpl *feed)
{
    if (feed->expireMode() != Feed::OverrideUpdateMode) {
        return;
    }
    m_worker->runInDatabaseThread(&FeedDatabase::updateFeedExpireAge, feed->id(), feed->expireAge());
}

void StorageImpl::listenForChanges(FeedImpl *feed)
{
    QObject::connect(feed, &Feed::lastUpdateChanged, this, [this, feed] {
        m_worker->runInDatabaseThread(&FeedDatabase::updateFeedLastUpdate, feed->id(), feed->lastUpdate());
    });
    QObject::connect(feed, &Feed::updateIntervalChanged, this, [this, feed] {
        onUpdateIntervalChanged(feed);
    });
    QObject::connect(feed, &Feed::updateModeChanged, this, [this, feed] {
        onUpdateModeChanged(feed);
    });
    QObject::connect(feed, &Feed::expireModeChanged, this, [this, feed] {
        onExpireModeChanged(feed);
    });
    QObject::connect(feed, &Feed::expireAgeChanged, this, [this, feed] {
        onExpireAgeChanged(feed);
    });
    QObject::connect(feed, &Feed::nameChanged, this, [this, feed] {
        m_worker->runInDatabaseThread(&FeedDatabase::updateFeedName, feed->id(), feed->name());
    });
    QObject::connect(feed, &Feed::urlChanged, this, [this, feed] {
        m_worker->runInDatabaseThread(&FeedDatabase::updateFeedUrl, feed->id(), feed->url());
    });
    QObject::connect(feed, &Feed::categoryChanged, this, [this, feed] {
        m_worker->runInDatabaseThread(&FeedDatabase::updateFeedCategory, feed->id(), feed->category());
    });
    QObject::connect(feed, &Feed::linkChanged, this, [this, feed] {
        m_worker->runInDatabaseThread(&FeedDatabase::updateFeedLink, feed->id(), feed->link().toString());
    });
    QObject::connect(feed, &Feed::iconChanged, this, [this, feed] {
        m_worker->runInDatabaseThread(&FeedDatabase::updateFeedIcon, feed->id(), feed->icon().toString());
    });
    QObject::connect(feed, &Feed::flagsChanged, this, [this, feed] {
        m_worker->runInDatabaseThread(&FeedDatabase::updateFeedFlags, feed->id(), feed->flags());
    });
    QObject::connect(feed, &Feed::deleteRequested, this, [this, feed] {
        onFeedRequestDelete(feed);
    });
}

void StorageImpl::expire(FeedImpl *feed, const QDateTime &olderThan)
{
    m_worker->runInDatabaseThread(&FeedDatabase::deleteItemsOlderThan, feed->id(), olderThan);
}

void StorageImpl::Worker::customEvent(QEvent *e)
{
    if (e->type() == static_cast<int>(CommitEvent)) {
        m_db.commitTransaction();
        m_hasTransaction = false;
        e->accept();
    } else {
        QObject::customEvent(e);
    }
}

template<typename Payload, typename Func>
QFuture<Payload> StorageImpl::Worker::runInDatabaseThread(Func func)
{
    QPromise<Payload> op;
    QFuture<Payload> future = op.future();
    QMetaObject::invokeMethod(this, [this, func, op = std::move(op)]() mutable {
        op.start();
        func(m_db, op);
        op.finish();
    });
    return future;
}

template<typename Func>
void StorageImpl::Worker::runInDatabaseThread(Func func)
{
    QMetaObject::invokeMethod(this, [this, func]() {
        func(m_db);
    });
}

template<typename Func, typename... Args>
void StorageImpl::Worker::runInDatabaseThread(Func func, Args... args)
{
    runInDatabaseThread([func, args...](auto &db) {
        (db.*func)(args...);
    });
}

template<typename Func>
void StorageImpl::Worker::runOnMainThread(Func func)
{
    QSemaphore sem;
    QMetaObject::invokeMethod(m_storage, [func, &sem]() {
        func();
        sem.release();
    });
    constexpr const int kMaxWaitMsecs = 100;
    while (!sem.tryAcquire(1, kMaxWaitMsecs)) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
    }
}
