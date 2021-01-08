#include "feedlistmodel.h"
#include <QString>
#include <QVector>
#include "context.h"
#include "qmlfeedref.h"
#include "allitemsfeed.h"

using namespace FeedCore;

struct FeedListEntry {
    QmlFeedRef feed;
    QString icon;
    int unreadCount;
};

class FeedListModel::PrivData {
public:
    FeedListModel *parent;
    Context *context = nullptr;
    QVector<FeedListEntry> feeds;

    PrivData(FeedListModel *parent) :
        parent{ parent }
    {}

    void addItem(const FeedRef &feed);
};

FeedListModel::FeedListModel(QObject *parent)
    : QAbstractListModel(parent),
      priv{ std::make_unique<PrivData>(this) }
{

}

FeedListModel::~FeedListModel()=default;

void FeedListModel::PrivData::addItem(const FeedRef &feed)
{
    FeedListEntry item = {
        .feed=QmlFeedRef(feed),
        .icon="feed-subscribe",
        .unreadCount=feed->unreadCount()
    };
    feeds << item;
}

Context *FeedListModel::context() const
{
    return priv->context;
}

void FeedListModel::setContext(FeedCore::Context *context)
{
    priv->context = context;
    emit contextChanged();
}

int FeedListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return priv->feeds.size();
}

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int indexRow = index.row();
    const FeedListEntry &entry { priv->feeds[indexRow] };

    switch(role){
        case Roles::Feed:
            return QVariant::fromValue(entry.feed.get());

        case Roles::Icon:
            return entry.icon;
    }

    return QVariant();
}


QHash<int, QByteArray> FeedListModel::roleNames() const
{
    return {
        {Roles::Feed, "feed"},
        {Roles::Icon, "icon"},
    };
}

void FeedListModel::classBegin()
{

}

void FeedListModel::componentComplete()
{
    QTimer::singleShot(0, this, [this]{
        Future<FeedRef> *q { priv->context->getFeeds() };
        QObject::connect(q, &BaseFuture::finished, this,
                         [this, q]{ onGetFeedsFinished(q); });
        QObject::connect(priv->context, &Context::feedAdded, this, &FeedListModel::onFeedAdded);
    });
}

void FeedListModel::onGetFeedsFinished(Future<FeedRef> *sender)
{
    beginResetModel();
    priv->feeds.clear();
    auto ref = FeedRef(new AllItemsFeed(priv->context, tr("All Items", "special feed name")));
    priv->addItem(ref);
    for (const auto &item : sender->result()){
        priv->addItem(item);
        priv->feeds[0].unreadCount  += item->unreadCount();
    }
    endResetModel();
}

void FeedListModel::onFeedAdded(const FeedRef &feed)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    priv->addItem(feed);
    endInsertRows();
}
