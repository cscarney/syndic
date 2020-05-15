#include <QSortFilterProxyModel>

#include "feedmanager.h"
#include "ttrssfeedsource.h"
#include "basicfeedstorage.h"
#include "itemmodel.h"
#include "feedlistmodel.h"

FeedManager::FeedManager(QObject *parent)
    : QObject(parent),
      m_source(std::make_unique<TTRSSFeedSource>()),
      m_storage(std::make_unique<BasicFeedStorage>()),
      m_feedList(new FeedListModel(this))
{    
    QObject::connect(m_source.get(), &FeedSource::foundContent, this, &FeedManager::receiveItem);
    QObject::connect(m_source.get(), &FeedSource::foundFeed, this, &FeedManager::receiveFeed);
    QObject::connect(m_source.get(), &FeedSource::feedRead, this, &FeedManager::clearUnread);

    auto feeds = m_storage->getFeeds();
    for (auto i=feeds.constBegin(); i!=feeds.constEnd(); ++i) {
        m_feedList->addFeed(*i);
    }
}

FeedManager::~FeedManager() = default;

QAbstractItemModel *FeedManager::getModel(std::optional<qint64> feedFilter, bool unreadOnly)
{
    qDebug("getting a model");
    auto *sortedModel = new QSortFilterProxyModel;
    auto *itemModel = new ItemModel(sortedModel, unreadOnly, feedFilter);
    sortedModel->setSourceModel(itemModel);
    sortedModel->setSortRole(ItemModel::Date);
    sortedModel->sort(0, Qt::DescendingOrder);

    auto storedItems = m_storage->getAll();
    for (auto const &storedItem : storedItems) {
        itemModel->addItem(storedItem);
    }
    QObject::connect(this, &FeedManager::itemAdded, itemModel, &ItemModel::addItem);
    QObject::connect(this,&FeedManager::itemChanged, itemModel,&ItemModel::updateItem);
    QObject::connect(itemModel, &ItemModel::itemMarked, this, &FeedManager::markItem);
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

void FeedManager::markItem(FeedSource::Item const &item, FeedSource::ItemFlag flag, bool value)
{
    m_source->setFlag(item.id, flag, value);
    auto newItem = item;
    if (flag == FeedSource::UNREAD) {
        newItem.isUnread = value;
    } else if (flag == FeedSource::STARRED) {
        newItem.isStarred = value;
    }
    m_storage->storeItem(newItem);
    itemChanged(newItem);
}

void FeedManager::setUnread(qint64 id, bool value)
{
    auto item = m_storage->getById(id);
    markItem(item, FeedSource::UNREAD, value);
}

void FeedManager::setStarred(qint64 id, bool value)
{
    auto item = m_storage->getById(id);
    markItem(item, FeedSource::STARRED, value);
}

void FeedManager::setAllUnread(QVariant const &feedFilter, bool value)
{
    bool haveFeedFilter = false;
    qint64 feedId = feedFilter.toLongLong(&haveFeedFilter);
    auto storedItems = m_storage->getAll();
    for (auto const &item : storedItems) {
        if ((!haveFeedFilter) || (item.feedId == feedId)) {
            markItem(item, FeedSource::UNREAD, value);
        }
    }
}

void FeedManager::addFeed(QUrl url)
{
    m_source->addFeed(url);
}

void FeedManager::receiveItem(FeedSource::Item const &item)
{
    auto storedItem = m_storage->getById(item.id);
    auto id = m_storage->storeItem(item);
    if (storedItem.id == 0) {
        storedItem = item;
        storedItem.id = id;
        itemAdded(storedItem);
    }
}

void FeedManager::receiveFeed(const FeedSource::Feed &feed)
{
    if (m_storage->storeFeed(feed)) {
        m_feedList->addFeed(feed);
    }
}

static inline bool isOlderThan(QDateTime candidate, QDateTime predicate)
{
    if (!predicate.isValid()) {
        return true;
    } else {
        return candidate < predicate;
    }
}

/* this function does NOT notify the FeedSource of changes
 *  and should ONLY be used when we are processing updates
 *  FROM the source */
void FeedManager::clearUnread(qint64 feedId, QDateTime olderThan)
{
    auto storedItems = m_storage->getAll();
    for (auto const &item : storedItems) {
        if ((item.feedId == feedId)
            && (isOlderThan(item.date, olderThan))
            && (item.isUnread)) {
            auto newItem = item;
            newItem.isUnread = false;
            m_storage->storeItem(newItem);
            itemChanged(newItem);
        }
    }
}


