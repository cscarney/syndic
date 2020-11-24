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

QAbstractItemModel *FeedManager::getModel(std::optional<qint64> feedFilter, bool unreadOnly)
{
    qDebug("getting a model");
    auto *itemModel = new ItemModel(unreadOnly, feedFilter);
    itemModel->populate(priv->storage.get());
    itemModel->listenForUpdates(this);
    return itemModel->createSortedProxy();
}

QAbstractItemModel *FeedManager::getModel(QVariant const &feedFilter, bool unreadOnly)
{
    bool haveFeedFilter = false;
    auto feedId = feedFilter.toLongLong(&haveFeedFilter);
    auto feedOpt = haveFeedFilter ? std::make_optional(feedId) : std::nullopt;
    return getModel(feedOpt, unreadOnly);
}

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

void FeedManager::setAllRead(QVariant const &feedFilter, bool value)
{
    bool haveFeedFilter = false;
    qint64 feedId = feedFilter.toLongLong(&haveFeedFilter);
    auto filterOpt = haveFeedFilter ? std::optional(feedId) : std::nullopt;
    auto q = priv->storage->startItemQuery(filterOpt, true);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q, value](){
        for(auto const &item : q->result) {
            setRead(item.id, value);
        }
    });
}

void FeedManager::addFeed(QUrl url)
{
    // TODO implement this
    // m_source->addFeed(url);
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

void FeedManager::PrivData::populateFeedList() {
    auto *queryFeeds = storage->getFeeds();
    QObject::connect(queryFeeds, &FeedStorage::ItemQuery::finished, parent, [this, queryFeeds] {
        auto &feeds = queryFeeds->result;
        for (auto i=feeds.constBegin(); i!=feeds.constEnd(); ++i) {
            feedList->addFeed(*i);
            updateScheduler->schedule(i->id, i->headers.url, 60000, 0);
        }
    });
}


