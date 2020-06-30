#include <QSortFilterProxyModel>

#include "feedmanager.h"
#include "ttrssfeedsource.h"
#include "sqlitefeedstorage.h"
#include "itemmodel.h"
#include "feedlistmodel.h"

FeedManager::FeedManager(QObject *parent)
    : QObject(parent),
      m_source(std::make_unique<TTRSSFeedSource>()),
      m_storage(std::make_unique<SqliteFeedStorage>()),
      m_feedList(new FeedListModel(this))
{    
    QObject::connect(m_source.get(), &FeedSource::foundContent, this, &FeedManager::receiveItem);
    QObject::connect(m_source.get(), &FeedSource::foundFeed, this, &FeedManager::receiveFeed);

    auto *queryFeeds = m_storage->getFeeds();
    QObject::connect(queryFeeds, &FeedStorage::ItemQuery::finished, this, [this, queryFeeds] {
        auto &feeds = queryFeeds->result;
        for (auto i=feeds.constBegin(); i!=feeds.constEnd(); ++i) {
            m_feedList->addFeed(*i);
        }
    });
}

FeedManager::~FeedManager() = default;

static inline FeedStorage::ItemQuery *startQuery(FeedStorage &s, std::optional<qint64> feedFilter, bool unreadOnly)
{
    if (feedFilter) {
        if (unreadOnly) return s.getUnreadByFeed(*feedFilter);
        else return s.getByFeed(*feedFilter);
    } else {
        if (unreadOnly) return s.getUnread();
        else return s.getAll();
    }
}

QAbstractItemModel *FeedManager::getModel(std::optional<qint64> feedFilter, bool unreadOnly)
{
    qDebug("getting a model");
    auto *sortedModel = new QSortFilterProxyModel;
    auto *itemModel = new ItemModel(sortedModel, unreadOnly, feedFilter);
    sortedModel->setSourceModel(itemModel);
    sortedModel->setSortRole(ItemModel::Date);
    sortedModel->sort(0, Qt::DescendingOrder);

    auto *q = startQuery(*m_storage, feedFilter, unreadOnly);
    QObject::connect(q, &FeedStorage::ItemQuery::finished, this, [q, itemModel] {
        auto &storedItems = q->result;
        for (auto const &storedItem : storedItems) {
            // TODO what if the caller destroys the model before we populate it?
            itemModel->addItem(storedItem);
        }
    });

    QObject::connect(this, &FeedManager::itemAdded, itemModel, &ItemModel::addItem);
    QObject::connect(this,&FeedManager::itemChanged, itemModel,&ItemModel::updateItem);
    m_source->beginUpdate();
    return sortedModel;
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
    return m_feedList;
}

void FeedManager::setRead(qint64 id, bool value)
{
    auto *q = m_storage->updateItemRead(id, value);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q]{
        for (const auto &item : q->result) {
            itemChanged(item);
        }
    });
}

void FeedManager::setStarred(qint64 id, bool value)
{
    auto *q = m_storage->updateItemStarred(id, !value);
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
    auto q = startQuery(*m_storage, filterOpt, true);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this, q, value](){
        for(auto const &item : q->result) {
            setRead(item.id, value);
        }
    });
}

void FeedManager::addFeed(QUrl url)
{
    m_source->addFeed(url);
}

void FeedManager::receiveItem(FeedSource::Item const &item)
{
    auto *q = m_storage->storeItem(item);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this,q]{
        auto &result = q->result;
        for (auto item : result) itemAdded(item);
    });
}

void FeedManager::receiveFeed(FeedSource::Feed const &feed)
{
    auto *q = m_storage->storeFeed(feed);
    QObject::connect(q, &FeedStorageOperation::finished, this, [this,q]{
        auto &result = q->result;
        for (auto feed : result) {
            m_feedList->addFeed(feed);
        }
    });
}


