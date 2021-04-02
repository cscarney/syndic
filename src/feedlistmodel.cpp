#include "feedlistmodel.h"
#include <QString>
#include <QVector>
#include <QPointer>
#include "context.h"
#include "allitemsfeed.h"

using namespace FeedCore;

struct FeedListEntry {
    Feed *feed;
    QString icon;
};

class FeedListModel::PrivData {
public:
    FeedListModel *parent;
    Context *context = nullptr;
    QVector<FeedListEntry> feeds;

    PrivData(FeedListModel *parent) :
        parent{ parent }
    {}

    void addItem(FeedCore::Feed *feed);
    void removeItem(FeedCore::Feed *feed);
};

FeedListModel::FeedListModel(QObject *parent)
    : QAbstractListModel(parent),
      priv{ std::make_unique<PrivData>(this) }
{

}

FeedListModel::~FeedListModel()=default;

void FeedListModel::PrivData::addItem(FeedCore::Feed *feed)
{
    FeedListEntry item = {
        .feed=feed,
        .icon="feed-subscribe",
    };
    feeds << item;
    QObject::connect(feed, &QObject::destroyed, parent, [this, feed]{
        removeItem(feed);
    });
}

void FeedListModel::PrivData::removeItem(FeedCore::Feed *feed)
{
    int i = 0;
    while (i < feeds.length()) {
        const FeedListEntry &entry { feeds[i] };
        FeedCore::Feed *const candidate { entry.feed };
        if (candidate==feed) {
            parent->beginRemoveRows(QModelIndex(), i, i);
            feeds.remove(i);
            parent->endRemoveRows();
        } else {
            ++i;
        }
    }
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
            return QVariant::fromValue(entry.feed);

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
        Future<FeedCore::Feed*> *q { priv->context->getFeeds() };
        QObject::connect(q, &BaseFuture::finished, this,
                         [this, q]{ onGetFeedsFinished(q); });
        QObject::connect(priv->context, &Context::feedAdded, this, &FeedListModel::onFeedAdded);
    });
}

void FeedListModel::onGetFeedsFinished(Future<FeedCore::Feed*> *sender)
{
    beginResetModel();
    priv->feeds.clear();
    auto *allItems = new AllItemsFeed(priv->context, tr("All Items", "special feed name"), this);
    priv->addItem(allItems);
    for (const auto &item : sender->result()){
        priv->addItem(item);
    }
    endResetModel();
}

void FeedListModel::onFeedAdded(FeedCore::Feed *feed)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    priv->addItem(feed);
    endInsertRows();
}
