#include <QSortFilterProxyModel>
#include <QDebug>

#include "feedmanager.h"
#include "sqlitefeedstorage.h"
#include "itemmodel.h"
#include "feedlistmodel.h"
#include "updatescheduler.h"
#include "feedupdater.h"

struct FeedManager::PrivData {
    FeedManager *parent;
    std::unique_ptr<FeedStorage> storage;
    std::unique_ptr<UpdateScheduler> updateScheduler;
    FeedListModel *feedList;

    PrivData(FeedManager *parent);
    void populateFeedList();
};

FeedManager::FeedManager(QObject *parent)
    : QObject(parent),
      priv(std::make_unique<PrivData>(this))
{
    QObject::connect(priv->updateScheduler.get(), &UpdateScheduler::feedLoaded, this, &FeedManager::slotFeedLoaded);
    QObject::connect(priv->updateScheduler.get(), &UpdateScheduler::feedStatusChanged, this, &FeedManager::slotFeedStatusChanged);
    priv->populateFeedList();
    priv->updateScheduler->start();
}

FeedManager::~FeedManager() = default;

FeedManager::PrivData::PrivData(FeedManager *parent) :
    parent(parent),
    storage(std::make_unique<SqliteFeedStorage>()),
    updateScheduler(std::make_unique<UpdateScheduler>()),
    feedList(new FeedListModel(parent))
{ }

QAbstractItemModel *FeedManager::getFeedListModel()
{
    return priv->feedList;
}

void FeedManager::setRead(qint64 id, bool value)
{
    auto *q = priv->storage->updateItemRead(id, value);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        for (const auto &item : q->result) {
            itemChanged(item);
        }
    });
}

void FeedManager::setStarred(qint64 id, bool value)
{
    auto *q = priv->storage->updateItemStarred(id, !value);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        for (const auto &item : q->result) {
            itemChanged(item);
        }
    });
}

void FeedManager::addFeed(QUrl url)
{
    // TODO implement this
    // m_source->addFeed(url);
}

ItemQuery *FeedManager::startQuery(std::optional<qint64> feedFilter, bool unreadFilter)
{
    return priv->storage->startItemQuery(feedFilter, unreadFilter);
}

LoadStatus FeedManager::getFeedStatus(qint64 feedId)
{
    return priv->updateScheduler->getStatus(feedId);
}

void FeedManager::requestUpdate()
{
    priv->updateScheduler->updateAll();
}

void FeedManager::requestUpdate(qint64 feedId)
{
    priv->updateScheduler->update(feedId);
}

bool FeedManager::updatesInProgress()
{
    return priv->updateScheduler->updatesInProgress();
}

void FeedManager::slotFeedLoaded(FeedUpdater *updater, Syndication::FeedPtr content)
{
    qDebug() << "Got Feed " << content->title() << "\n";
    auto items = content->items();
    for (auto item : items) {
        auto *q = priv->storage->storeItem(updater->feedId(), item);
        QObject::connect(q, &FeedStorageOperation::finished, this, [this,q]{
            auto &result = q->result;
            for (auto item : result) itemAdded(item);
        });
    }
}

void FeedManager::slotFeedStatusChanged(FeedUpdater *updater, LoadStatus status)
{
    emit feedStatusChanged(updater->feedId(), status);
}

void FeedManager::PrivData::populateFeedList() {
    auto *queryFeeds = storage->getFeeds();
    QObject::connect(queryFeeds, &FeedStorageOperation::finished, parent, [this, queryFeeds] {
        auto &feeds = queryFeeds->result;
        for (auto i=feeds.constBegin(); i!=feeds.constEnd(); ++i) {
            feedList->addFeed(*i);
            updateScheduler->schedule(i->id, i->headers.url, 60000, 0);
        }
    });
}


