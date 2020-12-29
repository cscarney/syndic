#include "feedlistmodel.h"
#include <QString>
#include <QVector>
#include "context.h"
#include "qmlfeedref.h"
#include "allitemsfeed.h"

using namespace FeedCore;

struct FeedListEntry {
    FeedRef feed;
    QString icon;
    int unreadCount;
};

class FeedListModel::PrivData {
public:
    FeedListModel *parent;
    QVector<FeedListEntry> feeds;

    PrivData(FeedListModel *parent) :
        parent(parent)
    {}

    void addItem(const FeedRef &feed);
};

FeedListModel::FeedListModel(QObject *parent)
    : ManagedListModel(parent),
      priv(std::make_unique<PrivData>(this))
{

}

FeedListModel::~FeedListModel()=default;

void FeedListModel::PrivData::addItem(const FeedRef &feed)
{
    FeedListEntry item = {
        .feed=feed,
        .icon="feed-subscribe",
        .unreadCount=feed->unreadCount()
    };
    feeds << item;
}

void FeedListModel::initialize()
{
    Future<FeedRef> *q { manager()->startFeedQuery() };
    QObject::connect(q, &BaseFuture::finished, this,
                     [this, q]{ onFeedQueryFinished(q); });
    QObject::connect(manager(), &Context::feedAdded, this, &FeedListModel::onFeedAdded);
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
        case Roles::Ref:
            return QVariant::fromValue(QmlFeedRef(entry.feed));

        case Roles::Icon:
            return entry.icon;
    }

    return QVariant();
}


QHash<int, QByteArray> FeedListModel::roleNames() const
{
    return {
        {Roles::Ref, "feedRef"},
        {Roles::Icon, "icon"},
    };
}

void FeedListModel::onFeedQueryFinished(Future<FeedRef> *sender)
{
    beginResetModel();
    priv->feeds.clear();
    auto ref = FeedRef(new AllItemsFeed(manager(), tr("All Items", "special feed name")));
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
